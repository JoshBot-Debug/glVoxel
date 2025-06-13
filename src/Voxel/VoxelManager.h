#pragma once

#include <future>
#include <glm/glm.hpp>
#include <unordered_map>

#include "Voxel/Common.h"
#include "Voxel/HeightMap.h"
#include "Voxel/SparseVoxelOctree.h"

class VoxelManager {
  enum VoxelPalette {
    STONE = 0,
    DIRT = 1,
    GRASS = 2,
    SNOW = 3,
  };

private:
  int chunkSize;
  int chunkRadius;
  float worldStep;

  HeightMap *heightMap;

  std::vector<Voxel *> voxelPalette = {
      new Voxel(45, 45, 45, 255), new Voxel(101, 67, 33, 255),
      new Voxel(34, 139, 34, 255), new Voxel(255, 255, 255, 255)};

  std::mutex verticesMutex;
  std::vector<std::future<void>> futures;
  std::unordered_map<glm::ivec2, SparseVoxelOctree, IVec2Hash, IVec2Equal>
      chunks;

public:
  std::vector<Vertex> vertices;

public:
  VoxelManager(int chunkSize, int chunkRadius, float worldStep = 1.0f);
  ~VoxelManager();

  void setHeightMap(HeightMap *heightMap);

  const int getChunkRadius() const;

  const int getChunkSize() const;

  std::unordered_map<glm::ivec2, SparseVoxelOctree, IVec2Hash, IVec2Equal> &
  getChunks();

  SparseVoxelOctree &getChunk(const glm::ivec2 &coord);

  void meshChunk(const glm::ivec2 &coord);

  void generateChunk(const glm::ivec2 &coord);

  const std::vector<glm::ivec2>
  getChunkPositionsInRadius(const glm::ivec2 &center) const;

  const glm::ivec2 getChunkPosition(const glm::vec3 &position) const;
};