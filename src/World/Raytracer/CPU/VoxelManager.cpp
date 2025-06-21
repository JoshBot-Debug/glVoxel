#include "VoxelManager.h"
#include <execution>
#include <future>
#include <iostream>
#include <mutex>
#include <noise/noiseutils.h>
#include <string>
#include <unordered_set>

#include "Components.h"
#include "Debug.h"
#include "Voxel/GreedyMesh64.h"

using namespace RaytracerCPU;

VoxelManager::~VoxelManager() {
  for (auto &[voxel, from, to] : m_VoxelPalette)
    delete voxel;

  for (auto &[coord, tree] : m_Chunks)
    delete tree;
}

void VoxelManager::initialize(PerspectiveCamera *camera) { m_Camera = camera; }

void VoxelManager::update() {

  const glm::ivec3 currentChunkPosition = getChunkPosition(m_Camera->position);

  std::thread([this, currentChunkPosition]() {
    if (!m_UpdateMutex.try_lock())
      return;

    m_LastCameraPosition = m_Camera->position;
    raytrace(currentChunkPosition);

    CTextureBuffer *textureBuffer = m_Registry->get<CTextureBuffer>()[0];
    textureBuffer->flush();

    m_UpdateMutex.unlock();
  }).detach();

  if (m_PlayerChunkPosition != currentChunkPosition)
    std::thread([this, currentChunkPosition]() {
      if (!m_UpdateMutex.try_lock())
        return;

      m_PlayerChunkPosition = currentChunkPosition;

      std::vector<glm::ivec3> create =
          getChunkPositionsInRadius(currentChunkPosition);
      std::vector<glm::ivec3> remove = {};

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

      auto t1 = START_TIMER;

      std::for_each(std::execution::par, create.begin(), create.end(),
                    [this](auto &chunk) { generateChunk(chunk); });

      LOG("Chunks", create.size());
      END_TIMER(t1);

      raytrace(currentChunkPosition);

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

  // std::for_each(
  //     std::execution::par, m_VoxelPalette.begin(), m_VoxelPalette.end(),
  //     [this, tree, map](std::tuple<Voxel *, int, int> &t) {
  //       auto &[voxel, from, to] = t;

  //       uint64_t mask[(s_ChunkSize * s_ChunkSize) * (s_ChunkSize / 64)] =
  //       {0};

  //       for (int z = 0; z < s_ChunkSize; z++)
  //         for (int x = 0; x < s_ChunkSize; x++) {
  //           float n = map.GetValue(x, z);
  //           int height = static_cast<int>(std::round(
  //               (std::clamp(n, -1.0f, 1.0f) + 1) * (s_ChunkSize / 2)));
  //           for (int y = 0; y < height; y++) {
  //             int index = x + s_ChunkSize * (z + s_ChunkSize * y);
  //             if (y >= from && y < to)
  //               mask[index / 64] |= 1ULL << (index % 64);
  //           }
  //         }

  //       tree->set(mask, voxel);
  //     });

  auto &[voxel, from, to] = m_VoxelPalette[2];

  tree->set(1, 0, 0, voxel, 16);

  END_TIMER(t1);
}

void VoxelManager::raytrace(const glm::ivec3 &coord) {

  std::unique_lock lock(m_Mutex.get(coord));

  auto it = m_Chunks.find(coord);

  if (it == m_Chunks.end() || it->second == nullptr)
    return;

  auto t1 = START_TIMER;

  SparseVoxelOctree *tree = it->second;

  tree->setNeighbours(coord, m_Chunks);

  CTextureBuffer *textureBuffer = m_Registry->get<CTextureBuffer>()[0];

  textureBuffer->setDimension(m_Camera->viewportWidth,
                              m_Camera->viewportHeight);

  std::vector<uint32_t> &buffer = textureBuffer->getUpdateBuffer();

  const glm::ivec2 &dimension = textureBuffer->getDimension();
  const std::vector<int> &dx = textureBuffer->getDimensionXIter();
  const std::vector<int> &dy = textureBuffer->getDimensionYIter();

  std::for_each(
      std::execution::par, dy.begin(), dy.end(), [&, dimension](int y) {
        for (int x = 0; x < dimension.x; x++) {
          const int i = x + y * dimension.x;
          const glm::vec3 rayDirection = m_Camera->getRayDirection(x, y);
          Voxel *hitVoxel = tree->rayTrace(m_Camera->position, rayDirection);

          if (hitVoxel) {
            buffer[i] = hitVoxel->color;
          } else
            buffer[i] = 0x00000000;
        }
      });

  LOG_IVEC3("Raytraced", coord);
  END_TIMER(t1);
}

void VoxelManager::setHeightMap(HeightMap *heightMap) {
  m_HeightMap = heightMap;
}

void VoxelManager::setRegistry(Registry *registry) {
  m_Registry = registry;
  Entity *entity = m_Registry->createEntity("textureBuffer");
  m_TextureBuffer = entity->add<CTextureBuffer>();
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