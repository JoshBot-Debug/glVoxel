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
  enum class Type : unsigned char
  {
    NONE = 0,
    GRASS = 1,
    DIRT = 2,
    STONE = 3,
    SNOW = 4,
    MAX_VALUE = 5,
  };

  class SparseVoxelOctree
  {

  public:
    struct Node
    {
      uint8_t depth = 0;
      Type voxelType = Type::NONE;
      Node *children[8] = {nullptr};

      Node() = default;
      Node(uint8_t depth) : depth(depth) {}
    };

  private:
    int size;
    int maxDepth;
    Node *root;
    Type lockedType = Type::NONE;

    void set(Node *node, int x, int y, int z, Type voxelType, int size);
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

    void set(glm::vec3 position, Type type);
    Node *get(glm::vec3 position);

    void set(int x, int y, int z, Type type);
    Node *get(int x, int y, int z);

    void clear();
    void greedyMesh(std::vector<Vertex> &vertices);

    void lock(Type type);
    void unlock();

    const size_t getTotalMemoryUsage() const;
  };
}
