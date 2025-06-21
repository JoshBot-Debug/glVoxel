#pragma once

#include <future>
#include <glm/glm.hpp>
#include <unordered_map>

#include "ECS/Entity.h"

#include "Engine/Camera/PerspectiveCamera.h"

#include "Voxel/Common.h"
#include "Voxel/HeightMap.h"
#include "Voxel/SparseVoxelOctree.h"

#include "Components.h"
#include "Utility/IVecMutex.h"

namespace RaytracerCPU {

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
  Registry *m_Registry = nullptr;

  HeightMap *m_HeightMap = nullptr;

  CTextureBuffer *m_TextureBuffer = nullptr;

  PerspectiveCamera *m_Camera = nullptr;

  glm::vec3 m_LastCameraPosition{-999, -999, -999};
  glm::ivec3 m_PlayerChunkPosition{-999, -999, -999};

  std::vector<std::tuple<Voxel *, int, int>> m_VoxelPalette = {
      std::tuple{new Voxel(45, 45, 45, 255), 0, 16},
      std::tuple{new Voxel(101, 67, 33, 255), 16, 24},
      std::tuple{new Voxel(34, 139, 34, 255), 24, 64},
      std::tuple{new Voxel(255, 255, 255, 255), 64, 128},
  };

  IVecMutex m_Mutex;
  std::mutex m_UpdateMutex;
  std::shared_mutex m_SharedUpdateMutex;
  std::unordered_map<glm::ivec3, SparseVoxelOctree *> m_Chunks;

public:
  VoxelManager() = default;
  ~VoxelManager();

  void setHeightMap(HeightMap *heightMap);

  void setRegistry(Registry *registry);

  void initialize(PerspectiveCamera *camera);

  void update();

  void generateChunk(const glm::ivec3 &coord);

  void raytrace(const glm::ivec3 &coord);

  const std::vector<glm::ivec3>
  getChunkPositionsInRadius(const glm::ivec3 &center) const;

  const glm::ivec3 getChunkPosition(const glm::vec3 &position) const;
};

}; // namespace RaytracerCPU