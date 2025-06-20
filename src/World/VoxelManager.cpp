#include "VoxelManager.h"
#include <future>
#include <iostream>
#include <mutex>
#include <noise/noiseutils.h>
#include <unordered_set>

#include "Components.h"
#include "Debug.h"

VoxelManager::~VoxelManager() {
  for (Voxel *voxel : m_VoxelPalette)
    delete voxel;

  for (auto &[coord, tree] : m_Chunks)
    delete tree;
}

void VoxelManager::initialize(const glm::vec3 &position) {
  m_PlayerChunkPosition = getChunkPosition(position);
  generateTerrain(getChunkPositionsInRadius(getChunkPosition(position)));
}

void VoxelManager::update(const glm::vec3 &position) {
  const glm::ivec3 currentChunkPosition = getChunkPosition(position);

  if (m_PlayerChunkPosition == currentChunkPosition)
    return;

  std::thread([this, currentChunkPosition]() {
    if (!m_UpdateMutex.try_lock())
      return;

    m_PlayerChunkPosition = currentChunkPosition;

    std::vector<glm::ivec3> create =
        getChunkPositionsInRadius(currentChunkPosition);
    std::vector<glm::ivec3> remove;

    for (auto it = m_Chunks.begin(); it != m_Chunks.end();) {
      auto vit = std::find(create.begin(), create.end(), it->first);
      if (vit == create.end()) {
        std::unique_lock lock(m_Mutex.get(it->first));
        LOG_IVEC3("deleted", it->first);
        remove.push_back(it->first);
        delete it->second;
        it = m_Chunks.erase(it);
      } else {
        create.erase(vit);
        ++it;
      }
    }

    for (CVoxelBuffer *voxelBuffer : m_Registry->get<CVoxelBuffer>())
      for (const auto &coord : remove)
        voxelBuffer->erase(coord);

    m_UpdateMutex.unlock();

    generateTerrain(create);
  }).detach();
}

void VoxelManager::setHeightMap(HeightMap *heightMap) {
  m_HeightMap = heightMap;
}
void VoxelManager::setRegistry(Registry *registry) {
  m_Registry = registry;
  Entity *entity = m_Registry->createEntity("VoxelBuffer");
  m_VoxelBuffer = entity->add<CVoxelBuffer>();
}

const std::vector<glm::ivec3>
VoxelManager::getChunkPositionsInRadius(const glm::ivec3 &center) const {
  std::vector<glm::ivec3> result;
  for (int dz = -s_ChunkRadius.z; dz <= s_ChunkRadius.z; dz++)
    for (int dx = -s_ChunkRadius.x; dx <= s_ChunkRadius.x; dx++)
      for (int dy = -s_ChunkRadius.y; dy <= s_ChunkRadius.y; dy++)
        result.emplace_back(center.x + dx, center.y + dy, center.z + dz);
  return result;
}

const glm::ivec3
VoxelManager::getChunkPosition(const glm::vec3 &position) const {
  return {static_cast<int>(
              std::floor(position.x / static_cast<float>(s_ChunkSize))),
          static_cast<int>(
              std::floor(position.y / static_cast<float>(s_ChunkSize))),
          static_cast<int>(
              std::floor(position.z / static_cast<float>(s_ChunkSize)))};
}

void VoxelManager::generateTerrain(const std::vector<glm::ivec3> &coords) {
  std::thread([this, coords = coords,
               m_Futures = std::move(m_Futures)]() mutable {
    if (!m_UpdateMutex.try_lock())
      return;

    auto t1 = START_TIMER;

    for (const auto &coord : coords)
      m_Futures.push_back(std::async(
          std::launch::async, &VoxelManager::generateChunk, this, coord));

    for (auto &f : m_Futures)
      f.get();

    m_Futures.clear();

    for (const auto &coord : coords)
      m_Futures.push_back(std::async(std::launch::async,
                                     &VoxelManager::meshChunk, this, coord));

    for (auto &f : m_Futures)
      f.get();

    for (CVoxelBuffer *voxelBuffer : m_Registry->get<CVoxelBuffer>())
      voxelBuffer->flush();

    LOG("Chunks", coords.size());
    END_TIMER(t1);

    m_UpdateMutex.unlock();
  }).detach();
}

void VoxelManager::generateChunk(const glm::ivec3 &coord) {
  if (coord.y != 0)
    return;

  std::unique_lock lock(m_Mutex.get(coord));

  auto t1 = START_TIMER;

  auto it = m_Chunks.find(coord);

  if (it == m_Chunks.end() || it->second == nullptr)
    m_Chunks[coord] = new SparseVoxelOctree(s_ChunkSize);

  SparseVoxelOctree *tree = m_Chunks.at(coord);

  tree->clear();

  utils::NoiseMap map = m_HeightMap->build(coord.x, coord.x + s_HeightMapStep,
                                           coord.z, coord.z + s_HeightMapStep);

  auto generateBlockChunks = [&](int thresholdFrom, int thresholdTo,
                                 Voxel *voxel) mutable {
    uint64_t mask[(s_ChunkSize * s_ChunkSize) * (s_ChunkSize / 64)] = {0};

    for (int z = 0; z < s_ChunkSize; z++)
      for (int x = 0; x < s_ChunkSize; x++) {
        float n = map.GetValue(x, z);
        int height = static_cast<int>(
            std::round((std::clamp(n, -1.0f, 1.0f) + 1) * (s_ChunkSize / 2)));
        for (int y = 0; y < height; y++) {
          int index = x + s_ChunkSize * (z + s_ChunkSize * y);
          if (y >= thresholdFrom && y < thresholdTo)
            mask[index / 64] |= 1ULL << (index % 64);
        }
      }

    tree->setBlock(mask, voxel);
  };

  generateBlockChunks(0, 16, m_VoxelPalette[VoxelPalette::STONE]);
  generateBlockChunks(16, 24, m_VoxelPalette[VoxelPalette::DIRT]);
  generateBlockChunks(24, 64, m_VoxelPalette[VoxelPalette::GRASS]);
  generateBlockChunks(64, 128, m_VoxelPalette[VoxelPalette::SNOW]);

  END_TIMER(t1);
}

void VoxelManager::meshChunk(const glm::ivec3 &coord) {
  std::shared_lock lock(m_Mutex.get(coord));

  auto it = m_Chunks.find(coord);

  if (it == m_Chunks.end() || it->second == nullptr)
    return;

  auto t1 = START_TIMER;

  it->second->setNeighbours(coord, m_Chunks);

  const std::vector<Voxel *> &filters = it->second->getUniqueVoxels();

  for (size_t i = 0; i < filters.size(); i++) {
    std::vector<Vertex> vertices;
    it->second->greedyMesh(vertices, filters[i]);
    Voxel *filter = filters[i];

    for (size_t j = 0; j < vertices.size(); j++) {
      vertices[j].x += static_cast<float>(coord.x * s_ChunkSize);
      vertices[j].y += static_cast<float>(coord.y * s_ChunkSize);
      vertices[j].z += static_cast<float>(coord.z * s_ChunkSize);
      vertices[j].color = filter->color;
      vertices[j].material = filter->material;
    }

    for (CVoxelBuffer *voxelBuffer : m_Registry->get<CVoxelBuffer>())
      voxelBuffer->setVertices(coord, vertices);
  }

  END_TIMER(t1);
}