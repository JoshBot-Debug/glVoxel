#include "Voxel.h"

Voxel::Voxel(uint32_t color) : color(color) {}

Voxel::Voxel(uint32_t color, uint32_t material) : color(color), material(material) {}

bool Voxel::operator==(const Voxel &other) const
{
  return this->color == other.color && this->material == other.material;
}

bool Voxel::operator!=(const Voxel &other) const
{
  return !(*this == other);
}

void Voxel::setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
  color = (r << 24) | (g << 16) | (b << 8) | a;
}