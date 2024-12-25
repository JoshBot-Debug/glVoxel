#pragma once

#include <bit>
#include <bitset>
#include <glm/glm.hpp>

class UniformGrid3D
{
private:
  static constexpr uint32_t SIZE = 64;

  static constexpr uint32_t BITS = sizeof(uint64_t) * 8;

  uint64_t gridBT[(SIZE * SIZE * SIZE) / BITS] = {};
  uint64_t gridLR[(SIZE * SIZE * SIZE) / BITS] = {};
  uint64_t gridFB[(SIZE * SIZE * SIZE) / BITS] = {};

public:
  uint32_t getValue(int x, int y, int z)
  {
    unsigned int btIndex = y + (SIZE * (x + (SIZE * z)));
    return (gridBT[btIndex / BITS] >> btIndex % BITS) & 1;
  }

  void setValue(int x, int y, int z, uint32_t value)
  {
    assert(value == 0 || value == 1);

    unsigned int btIndex = y + (SIZE * (x + (SIZE * z)));
    unsigned int lrIndex = x + (SIZE * (y + (SIZE * z)));
    unsigned int fbIndex = z + (SIZE * (x + (SIZE * y)));

    if (value == 1)
    {
      gridBT[btIndex / BITS] |= (1ULL << (btIndex % BITS));
      gridLR[lrIndex / BITS] |= (1ULL << (lrIndex % BITS));
      gridFB[fbIndex / BITS] |= (1ULL << (fbIndex % BITS));
    }
    else
    {
      gridBT[btIndex / BITS] &= ~(1ULL << (btIndex % BITS));
      gridLR[lrIndex / BITS] &= ~(1ULL << (lrIndex % BITS));
      gridFB[fbIndex / BITS] &= ~(1ULL << (fbIndex % BITS));
    }
  }

  uint32_t getValue(glm::ivec3 position)
  {
    return getValue(position.x, position.y, position.z);
  }

  void setValue(glm::ivec3 position, uint32_t value)
  {
    setValue(position.x, position.y, position.z, value);
  }

  const glm::ivec3 size()
  {
    return {SIZE, SIZE, SIZE};
  }

  uint64_t &getColumn(uint32_t x, uint32_t y, uint32_t z)
  {
    // y - the bits offset
    return gridBT[(SIZE * (x + (SIZE * z))) / BITS];
  }

  uint64_t &getRow(uint32_t x, uint32_t y, uint32_t z)
  {
    // x - the bits offset
    return gridLR[(SIZE * (y + (SIZE * z))) / BITS];
  }

  uint64_t &getDepth(uint32_t x, uint32_t y, uint32_t z)
  {
    // z - the bits offset
    return gridFB[(SIZE * (x + (SIZE * y))) / BITS];
  }
};