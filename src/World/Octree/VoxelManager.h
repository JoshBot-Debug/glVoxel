#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include "SparseVoxelOctree.h"
#include "World/HeightMap.h"
#include "World/Common.h"

#include <future>

class VoxelManager
{
private:
  int chunkSize;
  int chunkRadius;
  float worldStep;

  HeightMap *heightMap;

  std::mutex verticesMutex;
  std::vector<std::future<void>> chunkFutures;

  std::unordered_map<glm::ivec2, SparseVoxelOctree, IVec2Hash, IVec2Equal> chunks;

  void generateChunk(const glm::ivec2 &coord);

public:
  std::vector<Vertex> vertices;

public:
  VoxelManager(int chunkSize, int chunkRadius, float worldStep = 1.0f);

  void initialize(const glm::vec3 &position, HeightMap *heightMap);

  void update(const glm::vec3 &position);

  const std::vector<Vertex> &getVertices() const;

  const std::vector<glm::ivec2> getChunkPositionsInRadius(const glm::ivec2 &center) const;

  const glm::ivec2 getChunkPosition(const glm::vec3 &position) const;
};