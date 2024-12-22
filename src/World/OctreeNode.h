#pragma once

#include <vector>

template <typename T>
class OctreeNode
{
private:
  T data;
  std::vector<OctreeNode> children;

public:
  ~OctreeNode() {}

  T &getData() { return data; }

  std::vector<OctreeNode> &subdivide()
  {
    children.reserve(8);
    for (size_t i = 0; i < 8; i++)
      children.emplace_back();
    return children;
  }

  std::vector<T *> traverse()
  {
    std::vector<T *> nodes;

    for (const OctreeNode &child : children)
    {
      std::vector<OctreeNode> cNodes = child.traverse();
      nodes.insert(nodes.end(), cNodes.begin(), cNodes.end());
    }

    return nodes;
  }
};