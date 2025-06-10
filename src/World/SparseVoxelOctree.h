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
  struct Voxel
  {
    int color = 0;
    int material = 0;
    Voxel() = default;
    Voxel(int color) : color(color) {}
    Voxel(int color, int material) : color(color), material(material) {}
  };

  class SparseVoxelOctree
  {

  public:
    struct Node
    {
      uint8_t depth = 0;
      Voxel *voxel = nullptr;
      Node *children[8] = {nullptr};

      Node() = default;
      Node(uint8_t depth) : depth(depth) {}
      ~Node()
      {
        delete voxel;
        voxel = nullptr;
      }
    };

  private:
    int size;
    int maxDepth;
    Node *root;

    void set(Node *node, int x, int y, int z, int color, int size);
    Node *get(Node *node, int x, int y, int z, int size, Voxel *filter = nullptr);
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
    Node *get(glm::vec3 position, Voxel *filter = nullptr);

    void set(int x, int y, int z, int color);
    Node *get(int x, int y, int z, Voxel *filter = nullptr);

    void clear();
    void greedyMesh(std::vector<Vertex> &vertices, Voxel *filter = nullptr);

    const size_t getTotalMemoryUsage() const;
  };
}
