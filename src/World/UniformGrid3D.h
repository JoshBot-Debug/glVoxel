#pragma once

#include <bit>
#include <bitset>
#include <glm/glm.hpp>

class UniformGrid3D
{
private:
  static constexpr uint32_t SIZE = 64;

  static constexpr uint32_t BITS = sizeof(uint64_t) * 8;

  uint64_t grid[(SIZE * SIZE * SIZE) / BITS] = {};

public:
  uint32_t getIndex(int x, int y, int z) const
  {
    return y + (SIZE * (x + (SIZE * z)));
  }

  uint32_t getIndex(glm::ivec3 position) const
  {
    return getIndex(position.x, position.y, position.z);
  }

  glm::ivec3 getPosition(int index)
  {
    return {(index / SIZE) % SIZE, index % SIZE, index / (SIZE * SIZE)};
  }

  uint32_t getValue(uint32_t index)
  {
    return (grid[index / BITS] >> index % BITS) & 1;
  }

  void setValue(uint32_t index, uint32_t value)
  {
    assert(value == 0 || value == 1);
    if (value == 1)
      grid[index / BITS] |= (1ULL << (index % BITS));
    else
      grid[index / BITS] &= ~(1ULL << (index % BITS));
  }

  uint32_t getValue(int x, int y, int z)
  {
    return getValue(getIndex(x, y, z));
  }

  void setValue(int x, int y, int z, uint32_t value)
  {
    setValue(getIndex(x, y, z), value);
  }

  uint32_t getValue(glm::ivec3 position)
  {
    return getValue(getIndex(position));
  }

  void setValue(glm::ivec3 position, uint32_t value)
  {
    setValue(getIndex(position), value);
  }

  const glm::ivec3 size()
  {
    return {SIZE, SIZE, SIZE};
  }

  const uint32_t count() const
  {
    return SIZE * SIZE * SIZE;
  }

  uint64_t &getColumn(uint32_t x, uint32_t z)
  {
    const uint32_t columnIndex = getIndex(x, 0, z);

    return grid[columnIndex / BITS];
  }
};