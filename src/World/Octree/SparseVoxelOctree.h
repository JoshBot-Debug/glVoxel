#pragma once

#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>

#include <algorithm>
#include <execution>

#include "Engine/Types.h"
#include "Debug.h"
#include "Voxel.h"
#include "Node.h"

#include <fstream>

class SparseVoxelOctree
{
private:
  int size;
  int maxDepth;
  Node *root;
  std::vector<Voxel> uniqueVoxels;

  void set(Node *node, int x, int y, int z, Voxel *voxel, int size);
  Node *get(Node *node, int x, int y, int z, int size, int lod = -1, Voxel *filter = nullptr);
  void clear(Node *node);

  const size_t getMemoryUsage(Node *node) const;

public:
  SparseVoxelOctree();
  SparseVoxelOctree(int size);
  ~SparseVoxelOctree();

  const int getSize() const;
  const int getMaxDepth() const;
  Node *getRoot();

  void set(glm::vec3 position, Voxel *voxel);
  Node *get(glm::vec3 position, int maxDepth = -1, Voxel *filter = nullptr);

  void set(int x, int y, int z, Voxel *voxel);
  Node *get(int x, int y, int z, int maxDepth = -1, Voxel *filter = nullptr);

  void clear();
  void greedyMesh(std::vector<Vertex> &vertices, Voxel *filter = nullptr);

  const size_t getTotalMemoryUsage() const;

  std::vector<Voxel> getUniqueVoxels();
};