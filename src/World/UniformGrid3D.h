#pragma once

#include <glm/glm.hpp>
#include <bitset>

template <const glm::ivec3 size>
class UniformGrid3D
{
private:
  glm::ivec3 center = {size.x / 2, size.y / 2, size.z / 2};
  std::bitset<size.x> grid[size.y * size.z];

public:
  unsigned int getIndex(unsigned int x, unsigned int y, unsigned int z) const
  {
    return (x + center.x) + (size.x * ((y + center.y) + (size.y * (z + center.z))));
  }

  glm::vec3 getPosition(int index)
  {
    return {(index % size.x) - center.x, ((index / size.x) % size.y) - center.y, ((index / (size.x * size.y)) % size.z) - center.z};
  }

  unsigned int getValue(unsigned int index)
  {
    unsigned int bitIndex = index % size.x;
    unsigned int gridIndex = index / size.x;
    return grid[gridIndex].test(bitIndex);
  }

  unsigned int getValue(unsigned int x, unsigned int y, unsigned int z)
  {
    return getValue(getIndex(x, y, z));
  }

  void setValue(unsigned int x, unsigned int y, unsigned int z, unsigned int value)
  {
    setValue(getIndex(x, y, z), value);
  }

  void setValue(unsigned int index, unsigned int value)
  {
    unsigned int bitIndex = index % size.x;
    unsigned int gridIndex = index / size.x;
    grid[gridIndex][bitIndex] = value;
  }

  const glm::ivec3 getSize() const
  {
    return size;
  }

  const unsigned int getCount() const
  {
    return size.x * size.y * size.z;
  }
};