#pragma once
#include "Voxel/Voxel.h"
#include <glm/glm.hpp>

struct Node
{
  uint8_t depth = 0;
  Voxel *voxel = nullptr;
  Node *children[8] = {nullptr};

  Node();
  Node(uint8_t depth);
  ~Node();

  bool operator==(const Node &other) const;
  bool operator!=(const Node &other) const;

  void clear();

  Voxel *getAverageVoxel();
};
