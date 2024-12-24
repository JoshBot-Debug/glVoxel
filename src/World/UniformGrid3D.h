#pragma once

#include <bit>
#include <bitset>
#include <glm/glm.hpp>

class UniformGrid3D
{
private:
  static constexpr unsigned int SIZE = 64;

  static constexpr unsigned int BITS = sizeof(unsigned long long) * 8;

  unsigned long long grid[(SIZE * SIZE * SIZE) / BITS] = {};

public:
  unsigned int getIndex(int x, int y, int z) const
  {
    return y + (SIZE * (x + (SIZE * z)));
  }

  unsigned int getIndex(glm::ivec3 position) const
  {
    return getIndex(position.x, position.y, position.z);
  }

  glm::ivec3 getPosition(int index)
  {
    return {(index / SIZE) % SIZE, index % SIZE, index / (SIZE * SIZE)};
  }

  unsigned int getValue(unsigned int index)
  {
    return (grid[index / BITS] >> index % BITS) & 1;
  }

  void setValue(unsigned int index, unsigned int value)
  {
    assert(value == 0 || value == 1);
    if (value == 1)
      grid[index / BITS] |= (1ULL << (index % BITS));
    else
      grid[index / BITS] &= ~(1ULL << (index % BITS));
  }

  unsigned int getValue(int x, int y, int z)
  {
    return getValue(getIndex(x, y, z));
  }

  void setValue(int x, int y, int z, unsigned int value)
  {
    setValue(getIndex(x, y, z), value);
  }

  unsigned int getValue(glm::ivec3 position)
  {
    return getValue(getIndex(position));
  }

  void setValue(glm::ivec3 position, unsigned int value)
  {
    setValue(getIndex(position), value);
  }

  const glm::ivec3 size()
  {
    return {SIZE, SIZE, SIZE};
  }

  const unsigned int count() const
  {
    return SIZE * SIZE * SIZE;
  }

  unsigned long long &getColumn(unsigned int x, unsigned int z)
  {
    const unsigned int columnIndex = getIndex(x, 0, z);

    return grid[columnIndex / BITS];
  }
};