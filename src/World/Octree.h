#pragma once

#include <vector>

template <typename Node>
class Octree
{
private:
  Node node;
  std::vector<Octree> children;

public:
  ~Octree() {}

  Node &getNode() { return node; }

  std::vector<Octree> &subdivide()
  {
    children.reserve(8);
    for (size_t i = 0; i < 8; i++)
      children.emplace_back();
    return children;
  }
};