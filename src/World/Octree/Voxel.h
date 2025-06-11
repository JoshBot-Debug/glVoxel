#pragma once
#include <glm/glm.hpp>

struct Voxel
{
  uint32_t color = 0;
  uint32_t material = 0;

  Voxel() = default;
  Voxel(uint32_t color);
  Voxel(uint32_t color, uint32_t material);

  bool operator==(const Voxel &other) const;
  bool operator!=(const Voxel &other) const;

  void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
};