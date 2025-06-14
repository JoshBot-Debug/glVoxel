#pragma once

#include <algorithm>
#include <execution>
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

#include "Debug.h"
#include "Engine/Types.h"
#include "Voxel/Common.h"
#include "Voxel/Node.h"
#include "Voxel/Voxel.h"

class SparseVoxelOctree {
private:
  int size;
  int maxDepth;
  glm::ivec3 treePosition{0, 0, 0};

  Node *root = new Node(0);

  std::vector<Voxel *> uniqueVoxels;
  std::unordered_map<glm::ivec3, SparseVoxelOctree *> neighbours;

  void setBlock(uint64_t (&mask)[], int x, int y, int z, Voxel *voxel,
                int scale);

  void set(Node *node, int x, int y, int z, Voxel *voxel, int size,
           int maxSize = 1);
  Node *get(Node *node, int x, int y, int z, int size, int lod = -1,
            Voxel *filter = nullptr);
  void clear(Node *node);

  const size_t getMemoryUsage(Node *node) const;

public:
  SparseVoxelOctree();
  SparseVoxelOctree(int size);
  ~SparseVoxelOctree();

  const int getSize() const;
  const int getMaxDepth() const;
  Node *getRoot();

  /**
   * Requires you to pass in a mask
   *
   * const int size = 256;
   * uint64_t mask[size * size * (size / 64)] = {0};
   *
   * for (int z = 0; z < size; z++)
   *   for (int x = 0; x < size; x++)
   *      for (int y= 0; y < size; y++)
   *      {
   *          int index = x + size * (z + size * y);
   *          if(blockIsGrass)
   *            mask[index / 64] |= 1UL << (index % 64);
   *      }
   *
   */
  void setBlock(uint64_t (&mask)[], Voxel *voxel);

  void set(glm::vec3 position, Voxel *voxel, int maxSize = 1);
  Node *get(glm::vec3 position, int maxDepth = -1, Voxel *filter = nullptr);

  void set(int x, int y, int z, Voxel *voxel, int maxSize = 1);
  Node *get(int x, int y, int z, int maxDepth = -1, Voxel *filter = nullptr);

  void clear();
  void greedyMesh(std::vector<Vertex> &vertices, Voxel *filter = nullptr);

  const size_t getTotalMemoryUsage() const;

  const std::vector<Voxel *> &getUniqueVoxels() const;

  void setNeighbours(
      const glm::ivec3 &treePosition,
      std::unordered_map<glm::ivec3, SparseVoxelOctree *> &neighbours);
};