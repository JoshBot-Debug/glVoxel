#include "VoxelManager.h"
#include <future>
#include <iostream>
#include <mutex>
#include <noise/noiseutils.h>
#include <unordered_set>

#include "Components.h"
#include "Debug.h"

VoxelManager::VoxelManager(int CHUNK_SIZE, int CHUNK_RADIUS,
                           float HEIGHT_MAP_STEP)
    : CHUNK_SIZE(CHUNK_SIZE), CHUNK_RADIUS(CHUNK_RADIUS),
      HEIGHT_MAP_STEP(HEIGHT_MAP_STEP) {}

VoxelManager::~VoxelManager() {
  for (Voxel *voxel : voxelPalette)
    delete voxel;
}

void VoxelManager::initialize(const glm::vec3 &position) {
  playerPosition = position;
  generateTerrain(getChunkPositionsInRadius(getChunkPosition(position)));
}

void VoxelManager::update(const glm::vec3 &position) {
  const glm::ivec3 currentCenter = getChunkPosition(position);

  if (playerPosition == currentCenter)
    return;

  playerPosition = currentCenter;

  const std::vector<glm::ivec3> coords =
      getChunkPositionsInRadius(currentCenter);

  for (auto it = chunks.begin(); it != chunks.end();) {
    if (std::find(coords.begin(), coords.end(), it->first) == coords.end()) {
      std::unique_lock lock(mutex.get(it->first));

      delete it->second;
      it->second = nullptr;
      it = chunks.erase(it);
    } else
      ++it;
  }

  std::vector<glm::ivec3> coordsToCreate;

  for (const auto &coord : coords)
    if (chunks.find({coord.x, coord.y, coord.z}) == chunks.end())
      coordsToCreate.push_back(coord);

  generateTerrain(coordsToCreate);
}

void VoxelManager::setHeightMap(HeightMap *heightMap) {
  this->heightMap = heightMap;
}
void VoxelManager::setRegistry(Registry *registry) {
  this->registry = registry;
  Entity *entity = registry->createEntity("VoxelBuffer");
  this->voxelBuffer = entity->add<CVoxelBuffer>(false);
}

const std::vector<glm::ivec3>
VoxelManager::getChunkPositionsInRadius(const glm::ivec3 &center) const {
  std::vector<glm::ivec3> result;

  for (int dz = -CHUNK_RADIUS; dz < CHUNK_RADIUS; ++dz)
    for (int dx = -CHUNK_RADIUS; dx < CHUNK_RADIUS; ++dx)
      for (int dy = -CHUNK_RADIUS; dy < CHUNK_RADIUS; ++dy)
        result.emplace_back(center.x + dx, center.y + dy, center.z + dz);

  return result;
}

const glm::ivec3
VoxelManager::getChunkPosition(const glm::vec3 &position) const {
  return {static_cast<int>(std::floor(position.x / CHUNK_SIZE)),
          static_cast<int>(std::floor(position.y / CHUNK_SIZE)),
          static_cast<int>(std::floor(position.z / CHUNK_SIZE))};
}

void VoxelManager::generateTerrain(const std::vector<glm::ivec3> &coords) {
  for (const auto &coord : coords) {
    std::cout << "Coord: " << coord.x << " " << coord.y << " " << coord.z
              << std::endl;
    futures.push_back(std::async(std::launch::async,
                                 &VoxelManager::generateChunk, this, coord));
  }

  std::thread([this, coords = coords, futures = std::move(futures)]() mutable {
    auto t1 = START_TIMER;
    for (auto &f : futures)
      f.get();

    for (const glm::ivec3 &coord : coords) {
      if (chunks.contains({coord.x, coord.y, coord.z}))
        chunks[{coord.x, coord.y, coord.z}]->setNeighbours(coord, chunks);
    }

    futures.clear();

    for (const auto &coord : coords)
      futures.push_back(std::async(std::launch::async, &VoxelManager::meshChunk,
                                   this, coord));

    for (auto &f : futures)
      f.get();

    voxelBuffer->shouldUpdate = true;

    std::cout << "Chunks: " << coords.size() << std::endl;
    std::cout << "Size: " << CHUNK_SIZE << std::endl;
    END_TIMER(t1, "Chunks");
  })

      .detach();
}

void VoxelManager::generateChunk(const glm::ivec3 &coord) {
  std::unique_lock lock(mutex.get(coord));

  auto t1 = START_TIMER;

  auto [it, _] = chunks.try_emplace({coord.x, coord.y, coord.z},
                                    new SparseVoxelOctree(CHUNK_SIZE));

  SparseVoxelOctree *tree = it->second;

  tree->clear();

  if (coord.y != 0)
    return;

  utils::NoiseMap map =
      heightMap->build(coord.x + 1.0f, coord.x + HEIGHT_MAP_STEP + 1.0f,
                       coord.z + 1.0f, coord.z + HEIGHT_MAP_STEP + 1.0f);

  int stoneLimit =
      static_cast<int>(CHUNK_SIZE * heightMap->terrain.stoneThreshold);
  int dirtLimit =
      static_cast<int>(CHUNK_SIZE * heightMap->terrain.dirtThreshold);
  int grassLimit =
      static_cast<int>(CHUNK_SIZE * heightMap->terrain.grassThreshold);

  const unsigned int maskSize = CHUNK_SIZE * CHUNK_SIZE * (CHUNK_SIZE / 64);

  auto generateBlockChunks = [&](int thresholdFrom, int thresholdTo,
                                 Voxel *voxel) mutable {
    uint64_t mask[CHUNK_SIZE * CHUNK_SIZE * (CHUNK_SIZE / 64)] = {0};

    for (int z = 0; z < CHUNK_SIZE; z++)
      for (int x = 0; x < CHUNK_SIZE; x++) {
        float n = map.GetValue(x, z);
        unsigned int height = static_cast<unsigned int>(
            std::round((std::clamp(n, -1.0f, 1.0f) + 1) * (CHUNK_SIZE / 2)));
        for (int y = 0; y < height; y++) {
          int index = x + CHUNK_SIZE * (z + CHUNK_SIZE * y);
          if (y >= thresholdFrom && y < thresholdTo)
            mask[index / 64] |= 1UL << (index % 64);
        }
      }

    tree->setBlock(mask, voxel);
  };

  generateBlockChunks(0, stoneLimit, voxelPalette[VoxelPalette::STONE]);
  generateBlockChunks(stoneLimit, dirtLimit, voxelPalette[VoxelPalette::DIRT]);
  generateBlockChunks(dirtLimit, grassLimit, voxelPalette[VoxelPalette::GRASS]);
  generateBlockChunks(grassLimit, CHUNK_SIZE, voxelPalette[VoxelPalette::SNOW]);

  END_TIMER(t1, "generateChunk()");
}

void VoxelManager::meshChunk(const glm::ivec3 &coord) {
  std::shared_lock lock(mutex.get(coord));

  auto t1 = START_TIMER;

  if (!chunks.contains({coord.x, coord.y, coord.z}))
    return;

  SparseVoxelOctree *tree = chunks[{coord.x, coord.y, coord.z}];

  const std::vector<Voxel *> &filters = tree->getUniqueVoxels();

  std::vector<std::vector<Vertex>> tVertices;
  tVertices.resize(filters.size());

#pragma omp parallel for
  for (int i = 0; i < filters.size(); i++) {
    tree->greedyMesh(tVertices[i], filters[i]);

    Voxel *filter = filters[i];
    std::vector<Vertex> &iv = tVertices[i];

    for (int j = 0; j < iv.size(); j++) {
      iv[j].x += static_cast<float>(coord.x * CHUNK_SIZE);
      iv[j].y += static_cast<float>(coord.y * CHUNK_SIZE);
      iv[j].z += static_cast<float>(coord.z * CHUNK_SIZE);
      iv[j].color = filter->color;
      iv[j].material = filter->material;
    }

    {
      std::lock_guard<std::mutex> lock(verticesMutex);
      vertices.insert(vertices.end(), std::make_move_iterator(iv.begin()),
                      std::make_move_iterator(iv.end()));
    }
  }

  END_TIMER(t1, "meshChunk()");
}