#pragma once
#include <glm/glm.hpp>

struct Voxel {
  unsigned int color = 0;
  unsigned int material = 0;

  Voxel() = default;
  Voxel(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
  Voxel(unsigned int color);
  Voxel(unsigned int color, unsigned int material);

  bool operator==(const Voxel &other) const;
  bool operator!=(const Voxel &other) const;

  void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
};