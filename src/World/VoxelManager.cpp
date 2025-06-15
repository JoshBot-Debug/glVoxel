#include "VoxelManager.h"
#include <future>
#include <iostream>
#include <mutex>
#include <noise/noiseutils.h>
#include <unordered_set>

#include "Components.h"
#include "Debug.h"

VoxelManager::VoxelManager(int ChunkSize, glm::ivec3 ChunkRadius,
                           float HeightMapStep)
    : ChunkSize(ChunkSize), ChunkRadius(ChunkRadius),
      HeightMapStep(HeightMapStep) {}

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

  if (isUpdating.exchange(true)) {
    // Another update is still running
    std::cout << "Update skipped (already in progress)\n";
    return;
  }

  playerPosition = currentCenter;

  std::thread([this, currentCenter]() {
    const std::vector<glm::ivec3> coords =
        getChunkPositionsInRadius(currentCenter);

    for (auto it = chunks.begin(); it != chunks.end();) {
      if (std::find(coords.begin(), coords.end(), it->first) == coords.end()) {
        glm::ivec3 coord = it->first;
        std::unique_lock lock(mutex.get(coord));

        std::cout << "Delete SVO Lock: " << coord.x << " " << coord.y << " "
                  << coord.z << std::endl;

        delete it->second;
        it->second = nullptr;
        it = chunks.erase(it);

        std::cout << "Delete SVO Unlock: " << coord.x << " " << coord.y << " "
                  << coord.z << std::endl;
      } else {
        ++it;
      }
    }

    std::vector<glm::ivec3> create;

    for (const auto &coord : getChunkPositionsInRadius(currentCenter))
      if (!chunks.contains(coord))
        create.push_back(coord);

    generateTerrain(create);

    isUpdating.store(false);
  }).detach();
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

  for (int dz = -ChunkRadius.z; dz < ChunkRadius.z; ++dz)
    for (int dx = -ChunkRadius.x; dx < ChunkRadius.x; ++dx)
      for (int dy = -ChunkRadius.y; dy < ChunkRadius.y; ++dy)
        result.emplace_back(center.x + dx, center.y + dy, center.z + dz);

  return result;
}

const glm::ivec3
VoxelManager::getChunkPosition(const glm::vec3 &position) const {
  return {static_cast<int>(std::floor(position.x / ChunkSize)),
          static_cast<int>(std::floor(position.y / ChunkSize)),
          static_cast<int>(std::floor(position.z / ChunkSize))};
}

void VoxelManager::generateTerrain(const std::vector<glm::ivec3> &coords) {
  std::thread([this, coords = coords, futures = std::move(futures)]() mutable {
    auto t1 = START_TIMER;

    for (const auto &coord : coords) {
      std::cout << "Coord: " << coord.x << " " << coord.y << " " << coord.z
                << std::endl;
      futures.push_back(std::async(std::launch::async,
                                   &VoxelManager::generateChunk, this, coord));
    }

    for (auto &f : futures)
      f.get();

    futures.clear();

    for (const auto &coord : coords)
      futures.push_back(std::async(std::launch::async, &VoxelManager::meshChunk,
                                   this, coord));

    for (auto &f : futures)
      f.get();

    voxelBuffer->shouldUpdate = true;

    std::cout << "Chunks: " << coords.size() << std::endl;
    std::cout << "Size: " << ChunkSize << std::endl;
    END_TIMER(t1, "Chunks");
  }).detach();
}

void VoxelManager::generateChunk(const glm::ivec3 &coord) {
  if (coord.y != 0)
    return;

  std::unique_lock lock(mutex.get(coord));

  std::cout << "GC Write SVO Lock: " << coord.x << " " << coord.y << " "
            << coord.z << std::endl;

  auto t1 = START_TIMER;

  auto [it, _] = chunks.try_emplace(coord, new SparseVoxelOctree(ChunkSize));

  it->second->clear();

  utils::NoiseMap map =
      heightMap->build(coord.x + 1.0f, coord.x + HeightMapStep + 1.0f,
                       coord.z + 1.0f, coord.z + HeightMapStep + 1.0f);

  int stoneLimit =
      static_cast<int>(ChunkSize * heightMap->terrain.stoneThreshold);
  int dirtLimit =
      static_cast<int>(ChunkSize * heightMap->terrain.dirtThreshold);
  int grassLimit =
      static_cast<int>(ChunkSize * heightMap->terrain.grassThreshold);

  const unsigned int maskSize = ChunkSize * ChunkSize * (ChunkSize / 64);

  auto generateBlockChunks = [&](int thresholdFrom, int thresholdTo,
                                 Voxel *voxel) mutable {
    uint64_t mask[ChunkSize * ChunkSize * (ChunkSize / 64)] = {0};

    for (int z = 0; z < ChunkSize; z++)
      for (int x = 0; x < ChunkSize; x++) {
        float n = map.GetValue(x, z);
        unsigned int height = static_cast<unsigned int>(
            std::round((std::clamp(n, -1.0f, 1.0f) + 1) * (ChunkSize / 2)));
        for (int y = 0; y < height; y++) {
          int index = x + ChunkSize * (z + ChunkSize * y);
          if (y >= thresholdFrom && y < thresholdTo)
            mask[index / 64] |= 1UL << (index % 64);
        }
      }

    it->second->setBlock(mask, voxel);
  };

  generateBlockChunks(0, stoneLimit, voxelPalette[VoxelPalette::STONE]);
  generateBlockChunks(stoneLimit, dirtLimit, voxelPalette[VoxelPalette::DIRT]);
  generateBlockChunks(dirtLimit, grassLimit, voxelPalette[VoxelPalette::GRASS]);
  generateBlockChunks(grassLimit, ChunkSize, voxelPalette[VoxelPalette::SNOW]);

  END_TIMER(t1, "generateChunk()");

  std::cout << "GC Write SVO Unlock: " << coord.x << " " << coord.y << " "
            << coord.z << std::endl;
}

void VoxelManager::meshChunk(const glm::ivec3 &coord) {
  std::shared_lock lock(mutex.get(coord));

  if (!chunks.contains(coord))
    return;

  std::cout << "MC Read SVO Lock: " << coord.x << " " << coord.y << " "
            << coord.z << std::endl;

  auto t1 = START_TIMER;

  SparseVoxelOctree *tree = chunks.at(coord);

  tree->setNeighbours(coord, chunks);

  const std::vector<Voxel *> &filters = tree->getUniqueVoxels();

  std::vector<std::vector<Vertex>> tVertices;
  tVertices.resize(filters.size());

#pragma omp parallel for
  for (int i = 0; i < filters.size(); i++) {
    tree->greedyMesh(tVertices[i], filters[i]);

    Voxel *filter = filters[i];
    std::vector<Vertex> &iv = tVertices[i];

    for (int j = 0; j < iv.size(); j++) {
      iv[j].x += static_cast<float>(coord.x * ChunkSize);
      iv[j].y += static_cast<float>(coord.y * ChunkSize);
      iv[j].z += static_cast<float>(coord.z * ChunkSize);
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

  std::cout << "MC Read SVO Unlock: " << coord.x << " " << coord.y << " "
            << coord.z << std::endl;
}