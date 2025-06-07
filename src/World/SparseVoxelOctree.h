#pragma once

#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>

#include <algorithm>
#include <execution>

#include "Voxel.h"
#include "Utility.h"
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
      uint8_t depth = 0;
      Type type = Type::NONE;
      Node *children[8] = {nullptr};

      Node() = default;
      Node(uint8_t depth) : depth(depth) {}
    };

  private:
    int size;
    int maxDepth;
    Node *root;

    void set(Node *node, int x, int y, int z, Type type, int size);
    Node *get(Node *node, int x, int y, int z, int size);
    void clear(Node *node);

    const size_t getMemoryUsage(Node *node) const;

  public:
    SparseVoxelOctree();
    SparseVoxelOctree(int size);
    ~SparseVoxelOctree();

    int getSize();
    int getMaxDepth();
    Node *getRoot();

    void set(glm::vec3 position, Type type);
    Node *get(glm::vec3 position);

    void set(int x, int y, int z, Type type);
    Node *get(int x, int y, int z);

    void clear();
    void greedyMesh(std::vector<Vertex> &vertices);

    const size_t getTotalMemoryUsage() const;
  };
}
