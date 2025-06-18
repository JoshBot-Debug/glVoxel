#pragma once

#include <future>
#include <glm/glm.hpp>
#include <unordered_map>

#include "ECS/Entity.h"

#include "Voxel/Common.h"
#include "Voxel/HeightMap.h"
#include "Voxel/SparseVoxelOctree.h"

#include "Components.h"
#include "IVecMutex.h"

class VoxelManager {
  enum VoxelPalette {
    STONE = 0,
    DIRT = 1,
    GRASS = 2,
    SNOW = 3,
  };

private:
  static constexpr int s_ChunkSize = 128;
  static constexpr double s_HeightMapStep = 1.0f;
  static constexpr glm::ivec3 s_ChunkRadius = glm::ivec3{0, 0, 0};

private:
  Registry *m_Registry{nullptr};

  HeightMap *m_HeightMap{nullptr};

  CVoxelBuffer *m_VoxelBuffer{nullptr};

  glm::ivec3 m_PlayerChunkPosition{0, 0, 0};

  std::vector<Voxel *> m_VoxelPalette = {
      new Voxel(45, 45, 45, 255), new Voxel(101, 67, 33, 255),
      new Voxel(34, 139, 34, 255), new Voxel(255, 255, 255, 255)};

  IVecMutex m_Mutex;
  std::mutex m_UpdateMutex;
  std::vector<std::future<void>> m_Futures;
  std::unordered_map<glm::ivec3, SparseVoxelOctree *> m_Chunks;

public:
  VoxelManager() = default;
  ~VoxelManager();

  void setHeightMap(HeightMap *heightMap);

  void setRegistry(Registry *registry);

  void initialize(const glm::vec3 &position);

  void update(const glm::vec3 &position);

  void generateTerrain(const std::vector<glm::ivec3> &coords);

  void meshChunk(const glm::ivec3 &coord);

  void generateChunk(const glm::ivec3 &coord);

  const std::vector<glm::ivec3>
  getChunkPositionsInRadius(const glm::ivec3 &center) const;

  const glm::ivec3 getChunkPosition(const glm::vec3 &position) const;
};