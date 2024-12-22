#pragma once

#include <bit>
#include <bitset>
#include <glm/glm.hpp>

template <const glm::ivec3 size>
class UniformGrid3D
{
private:
  glm::ivec3 center = {size.x / 2, size.y / 2, size.z / 2};
  std::vector<std::vector<std::bitset<size.z>>> grid;

public:
  UniformGrid3D()
  {
    grid.resize(size.x);
    for (size_t x = 0; x < size.x; ++x)
      grid[x].resize(size.y);
  }

  unsigned int getIndex(int x, int y, int z) const
  {
    return (x + center.x) + (size.x * ((y + center.y) + (size.y * (z + center.z))));
  }

  glm::vec3 getPosition(int index)
  {
    int x = index % size.x;
    int y = (index / size.x) % size.y;
    int z = (index / (size.x * size.y)) % size.z;

    return {x - center.x, y - center.y, z - center.z};
  }

  unsigned int getValue(unsigned int index)
  {
    unsigned int x = index % size.x;
    unsigned int y = (index / size.x) % size.y;
    unsigned int z = (index / (size.x * size.y)) % size.z;
    return grid[x][y].test(z);
  }

  void setValue(unsigned int index, unsigned int value)
  {
    unsigned int x = index % size.x;
    unsigned int y = (index / size.x) % size.y;
    unsigned int z = (index / (size.x * size.y)) % size.z;
    grid[x][y][z] = value;
  }

  unsigned int getValue(int x, int y, int z)
  {
    return grid[x + center.x][y + center.y].test(z + center.z);
  }

  void setValue(int x, int y, int z, unsigned int value)
  {
    return grid[x + center.x][y + center.y][z + center.z] = value;
  }

  const glm::ivec3 &getSize() const
  {
    return size;
  }

  const glm::ivec3 &getCenter() const
  {
    return center;
  }

  const unsigned int getCount() const
  {
    return size.x * size.y * size.z;
  }

  const std::vector<std::vector<std::bitset<size.z>>> &getGrid()
  {
    return grid;
  };

  const std::bitset<size.z> &getRow(int x, int y)
  {
    return grid[x + center.x][y + center.y];
  }

  int leading1s(int x, int y)
  {
    const auto &bitset = grid[x][y];
    unsigned long long value = bitset.to_ullong();         // Convert bitset to an integer
    unsigned long long mask = (1ULL << bitset.size()) - 1; // Create a mask of all 1s (e.g., 1111111111111111)
    value ^= mask;                                         // Flip all bits (1 -> 0, 0 -> 1)
    return std::countl_zero(value) - (sizeof(value) * 8 - bitset.size());
  }
};