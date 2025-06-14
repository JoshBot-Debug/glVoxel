#include "Node.h"
#include <unordered_map>

Node::Node() {}

Node::Node(uint8_t depth) : depth(depth) {}

Node::~Node() { clear(); }

bool Node::operator==(const Node &other) const {
  return voxel->color == other.voxel->color &&
         voxel->material == other.voxel->material;
}

bool Node::operator!=(const Node &other) const { return !(*this == other); }

void Node::clear() {
  depth = 0;
  voxel = nullptr;

  for (int i = 0; i < 8; i++)
    if (children[i]) {
      delete children[i];
      children[i] = nullptr;
    }
}

Voxel *Node::getAverageVoxel() {
  Voxel *merged = new Voxel();

  std::unordered_map<int, int> color;
  std::unordered_map<int, int> material;

  for (const Node *child : children) {
    if (!child || !child->voxel)
      continue;

    color[child->voxel->color]++;
    material[child->voxel->material]++;
  }

  int colorCount = 0;
  for (const auto &[color, count] : color) {
    if (count < colorCount)
      continue;
    merged->color = color;
    colorCount = count;
  }

  int materialCount = 0;
  for (const auto &[material, count] : material) {
    if (count < materialCount)
      continue;
    merged->material = material;
    materialCount = count;
  }

  return merged;
}