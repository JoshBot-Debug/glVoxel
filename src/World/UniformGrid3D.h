#pragma once

#include <bit>
#include <bitset>
#include <glm/glm.hpp>

class UniformGrid3D
{
private:
  static constexpr uint32_t SIZE = 32;

  static constexpr uint32_t BITS = sizeof(uint32_t) * 8;

  uint32_t gridBT[(SIZE * SIZE * SIZE) / BITS] = {};
  uint32_t gridLR[(SIZE * SIZE * SIZE) / BITS] = {};
  uint32_t gridFB[(SIZE * SIZE * SIZE) / BITS] = {};

public:
  unsigned int getValue(int x, int y, int z)
  {
    unsigned int btIndex = y + (SIZE * (x + (SIZE * z)));
    return (gridBT[btIndex / BITS] >> btIndex % BITS) & 1;
  }

  void setValue(int x, int y, int z, unsigned int value)
  {
    assert(value == 0 || value == 1);

    unsigned int btIndex = y + (SIZE * (x + (SIZE * z)));
    unsigned int lrIndex = x + (SIZE * (y + (SIZE * z)));
    unsigned int fbIndex = z + (SIZE * (x + (SIZE * y)));

    if (value == 1)
    {
      gridBT[btIndex / BITS] |= (1U << (btIndex % BITS));
      gridLR[lrIndex / BITS] |= (1U << (lrIndex % BITS));
      gridFB[fbIndex / BITS] |= (1U << (fbIndex % BITS));
    }
    else
    {
      gridBT[btIndex / BITS] &= ~(1U << (btIndex % BITS));
      gridLR[lrIndex / BITS] &= ~(1U << (lrIndex % BITS));
      gridFB[fbIndex / BITS] &= ~(1U << (fbIndex % BITS));
    }
  }

  unsigned int getValue(glm::ivec3 position)
  {
    return getValue(position.x, position.y, position.z);
  }

  void setValue(glm::ivec3 position, unsigned int value)
  {
    setValue(position.x, position.y, position.z, value);
  }

  const glm::ivec3 size()
  {
    return {SIZE, SIZE, SIZE};
  }

  uint32_t &getColumn(unsigned int x, unsigned int y, unsigned int z)
  {
    // y - the bits offset
    return gridBT[(SIZE * (x + (SIZE * z))) / BITS];
  }

  void setColumn(unsigned int x, unsigned int y, unsigned int z, uint32_t value)
  {
    gridBT[(SIZE * (x + (SIZE * z))) / BITS] = value;
  }

  uint32_t &getRow(unsigned int x, unsigned int y, unsigned int z)
  {
    // x - the bits offset
    return gridLR[(SIZE * (y + (SIZE * z))) / BITS];
  }

  void setRow(unsigned int x, unsigned int y, unsigned int z, uint32_t value)
  {
    gridLR[(SIZE * (y + (SIZE * z))) / BITS] = value;
  }

  uint32_t &getDepth(unsigned int x, unsigned int y, unsigned int z)
  {
    // z - the bits offset
    return gridFB[(SIZE * (x + (SIZE * y))) / BITS];
  }

  void setDepth(unsigned int x, unsigned int y, unsigned int z, uint32_t value)
  {
    gridFB[(SIZE * (x + (SIZE * y))) / BITS] = value;
  }
};