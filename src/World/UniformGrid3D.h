#pragma once

#include <bitset>
#include <glm/glm.hpp>

template <glm::ivec3 GRID_SIZE>
class UniformGrid3D
{
private:
  glm::ivec3 center = {GRID_SIZE.x / 2, GRID_SIZE.y / 2, GRID_SIZE.z / 2};
  unsigned int grid[GRID_SIZE.x * GRID_SIZE.y * GRID_SIZE.z] = {};

public:
  unsigned int getIndex(int x, int y, int z) const
  {
    return (x + center.x) + (GRID_SIZE.x * ((y + center.y) + (GRID_SIZE.y * (z + center.z))));
  }

  unsigned int getIndex(glm::ivec3 position) const
  {
    return getIndex(position.x, position.y, position.z);
  }

  glm::ivec3 getPosition(int index)
  {
    int x = index % GRID_SIZE.x;
    int y = (index / GRID_SIZE.x) % GRID_SIZE.y;
    int z = (index / (GRID_SIZE.x * GRID_SIZE.y)) % GRID_SIZE.z;
    return {x - center.x, y - center.y, z - center.z};
  }

  unsigned int getValue(unsigned int index)
  {
    return grid[index];
  }

  void setValue(unsigned int index, unsigned int value)
  {
    grid[index] = value;
  }

  unsigned int getValue(int x, int y, int z)
  {
    return grid[getIndex(x, y, z)];
  }

  void setValue(int x, int y, int z, unsigned int value)
  {
    return grid[getIndex(x, y, z)] = value;
  }

  unsigned int getValue(glm::ivec3 position)
  {
    return grid[getIndex(position.x, position.y, position.z)];
  }

  void setValue(glm::ivec3 position, unsigned int value)
  {
    return grid[getIndex(position.x, position.y, position.z)] = value;
  }

  const glm::ivec3 size() const
  {
    return GRID_SIZE;
  }

  const unsigned int count() const
  {
    return GRID_SIZE.x * GRID_SIZE.y * GRID_SIZE.z;
  }

  const glm::ivec3 &getCenter() const
  {
    return center;
  }

  std::bitset<GRID_SIZE.y> getColumn(glm::ivec3 position)
  {
    std::bitset<GRID_SIZE.y> bits(0);

    for (size_t i = position.y + center.y; i < bits.size(); i++)
      bits[i] = grid[getIndex(position.x, i, position.z)];

    return bits;
  }
};