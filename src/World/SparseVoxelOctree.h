#pragma once

#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>

#include <algorithm>
#include <execution>

#include "Engine/Types.h"
#include "Debug.h"

#include <fstream>

namespace Voxel
{
  class SparseVoxelOctree
  {

  public:
    struct Node
    {
      int color = 0;
      int material = 0;
      uint8_t depth = 0;
      Node *children[8] = {nullptr};

      Node() = default;
      Node(uint8_t depth) : depth(depth) {}
    };

  private:
    int size;
    int maxDepth;
    Node *root;
    int lockedColor = 0;

    void set(Node *node, int x, int y, int z, int color, int size);
    Node *get(Node *node, int x, int y, int z, int size);
    void clear(Node *node);

    const size_t getMemoryUsage(Node *node) const;

  public:
    SparseVoxelOctree();
    SparseVoxelOctree(int size);
    ~SparseVoxelOctree();

    const int getSize() const;
    const int getMaxDepth() const;
    Node *getRoot();

    void set(glm::vec3 position, int color);
    Node *get(glm::vec3 position);

    void set(int x, int y, int z, int color);
    Node *get(int x, int y, int z);

    void clear();
    void greedyMesh(std::vector<Vertex> &vertices);

    void lock(int color);
    void unlock();

    const size_t getTotalMemoryUsage() const;
  };
}
