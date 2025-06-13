#include "VoxelManager.h"
#include <future>
#include <iostream>
#include <mutex>
#include <noise/noiseutils.h>
#include <unordered_set>

#include "Debug.h"

VoxelManager::VoxelManager(int chunkSize, int chunkRadius, float worldStep)
    : chunkSize(chunkSize), chunkRadius(chunkRadius), worldStep(worldStep) {}

VoxelManager::~VoxelManager() {
  for (Voxel *voxel : voxelPalette)
    delete voxel;
}

void VoxelManager::setHeightMap(HeightMap *heightMap) {
  this->heightMap = heightMap;
}

const int VoxelManager::getChunkRadius() const { return chunkRadius; }

const int VoxelManager::getChunkSize() const { return chunkSize; }

std::unordered_map<glm::ivec2, SparseVoxelOctree, IVec2Hash, IVec2Equal> &
VoxelManager::getChunks() {
  return chunks;
}

SparseVoxelOctree &VoxelManager::getChunk(const glm::ivec2 &coord) {
  return chunks[coord];
}

const std::vector<glm::ivec2>
VoxelManager::getChunkPositionsInRadius(const glm::ivec2 &center) const {
  std::vector<glm::ivec2> result;

  for (int dz = -chunkRadius; dz < chunkRadius; ++dz)
    for (int dx = -chunkRadius; dx < chunkRadius; ++dx)
      result.emplace_back(center.x + dx, center.y + dz);

  return result;
}

const glm::ivec2
VoxelManager::getChunkPosition(const glm::vec3 &position) const {
  return {static_cast<int>(std::floor(position.x / chunkSize)),
          static_cast<int>(std::floor(position.z / chunkSize))};
}

void VoxelManager::generateChunk(const glm::ivec2 &coord) {
  auto t1 = START_TIMER;
  auto [it, inserted] = chunks.try_emplace(coord, chunkSize);
  SparseVoxelOctree &tree = it->second;

  tree.clear();

  utils::NoiseMap map =
      heightMap->build(coord.x + 1.0f, coord.x + worldStep + 1.0f,
                       coord.y + 1.0f, coord.y + worldStep + 1.0f);

  const int size = tree.getSize();
  int stoneLimit = static_cast<int>(size * heightMap->terrain.stoneThreshold);
  int dirtLimit = static_cast<int>(size * heightMap->terrain.dirtThreshold);
  int grassLimit = static_cast<int>(size * heightMap->terrain.grassThreshold);

  const unsigned int maskSize = size * size * (size / 64);

  auto generateBlockChunks = [&](int thresholdFrom, int thresholdTo,
                                 Voxel *voxel) mutable {
    uint64_t mask[size * size * (size / 64)] = {0};

    for (int z = 0; z < size; z++)
      for (int x = 0; x < size; x++) {
        float n = map.GetValue(x, z);
        unsigned int height = static_cast<unsigned int>(
            std::round((std::clamp(n, -1.0f, 1.0f) + 1) * (size / 2)));
        for (int y = 0; y < height; y++) {
          int index = x + size * (z + size * y);
          if (y >= thresholdFrom && y < thresholdTo)
            mask[index / 64] |= 1UL << (index % 64);
        }
      }

    tree.setBlock(mask, voxel);
  };

  generateBlockChunks(0, stoneLimit, voxelPalette[VoxelPalette::STONE]);
  generateBlockChunks(stoneLimit, dirtLimit, voxelPalette[VoxelPalette::DIRT]);
  generateBlockChunks(dirtLimit, grassLimit, voxelPalette[VoxelPalette::GRASS]);
  generateBlockChunks(grassLimit, size, voxelPalette[VoxelPalette::SNOW]);

  END_TIMER(t1, "generateChunk()");
}

void VoxelManager::meshChunk(const glm::ivec2 &coord) {
  auto t1 = START_TIMER;

  SparseVoxelOctree &tree = chunks[coord];

  const std::vector<Voxel *> &filters = tree.getUniqueVoxels();

  std::vector<std::vector<Vertex>> tVertices;
  tVertices.resize(filters.size());

#pragma omp parallel for
  for (int i = 0; i < filters.size(); i++) {
    tree.greedyMesh(tVertices[i], filters[i]);

    Voxel *filter = filters[i];
    std::vector<Vertex> &iv = tVertices[i];

    for (int j = 0; j < iv.size(); j++) {
      iv[j].x += static_cast<float>(coord.x * tree.getSize());
      iv[j].z += static_cast<float>(coord.y * tree.getSize());
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