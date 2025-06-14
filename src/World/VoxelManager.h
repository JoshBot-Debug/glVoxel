#pragma once

#include <future>
#include <glm/glm.hpp>

#include <tuple>
#include <functional>
#include <unordered_map>

#include "ECS/Entity.h"

#include "Voxel/Common.h"
#include "Voxel/HeightMap.h"
#include "Voxel/SparseVoxelOctree.h"

#include "Components.h"

class VoxelManager
{
  enum VoxelPalette
  {
    STONE = 0,
    DIRT = 1,
    GRASS = 2,
    SNOW = 3,
  };

private:
  int CHUNK_SIZE;
  int CHUNK_RADIUS;
  float HEIGHT_MAP_STEP;

  Registry *registry{nullptr};

  HeightMap *heightMap{nullptr};

  CVoxelBuffer *voxelBuffer{nullptr};

  glm::ivec3 playerPosition{0, 0, 0};

  std::vector<Voxel *> voxelPalette = {
      new Voxel(45, 45, 45, 255), new Voxel(101, 67, 33, 255),
      new Voxel(34, 139, 34, 255), new Voxel(255, 255, 255, 255)};

  std::mutex verticesMutex;
  std::vector<std::future<void>> futures;
  std::unordered_map<std::tuple<int, int, int>, SparseVoxelOctree *> chunks;

public:
  std::vector<Vertex> vertices;

public:
  VoxelManager(int chunkSize, int chunkRadius, float worldStep = 1.0f);
  ~VoxelManager();

  void setHeightMap(HeightMap *heightMap);

  void setRegistry(Registry *registry);

  void initialize(const glm::vec3 &position);

  void update(const glm::vec3 &position);

  void generateTerrain(const std::vector<glm::ivec3> &coords);

  void meshChunk(const glm::ivec3 &coord);

  void generateChunk(const glm::ivec3 &coord);

  const std::vector<glm::ivec3> getChunkPositionsInRadius(const glm::ivec3 &center) const;

  const glm::ivec3 getChunkPosition(const glm::vec3 &position) const;
};