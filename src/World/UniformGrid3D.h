#pragma once

#include <glm/glm.hpp>
#include <immintrin.h>

class UniformGrid3D
{
public:
  static constexpr uint32_t SIZE = 32;
  static constexpr uint32_t BITS = sizeof(uint32_t) * 8;
  static constexpr uint32_t GRID_SIZE = (SIZE * SIZE * SIZE) / BITS;

private:
  uint32_t gx[GRID_SIZE] = {};
  uint32_t gy[GRID_SIZE] = {};
  uint32_t gz[GRID_SIZE] = {};

public:
  unsigned int get(int x, int y, int z)
  {
    if (x < 0 || y < 0 || z < 0 || x >= SIZE || y >= SIZE || z >= SIZE)
      return 0;

    unsigned int xi = x + (SIZE * (y + (SIZE * z)));
    return (gx[xi / BITS] >> xi % BITS) & 1;
  }

  void set(int x, int y, int z, unsigned int value)
  {
    assert(value == 0 || value == 1 && !(x < 0 || y < 0 || z < 0 || x >= SIZE || y >= SIZE || z >= SIZE));

    unsigned int xi = x + (SIZE * (y + (SIZE * z)));
    unsigned int yi = y + (SIZE * (x + (SIZE * z)));
    unsigned int zi = z + (SIZE * (y + (SIZE * x)));

    if (value == 1)
    {
      gx[xi / BITS] |= (1U << (xi % BITS));
      gy[yi / BITS] |= (1U << (yi % BITS));
      gz[zi / BITS] |= (1U << (zi % BITS));
    }
    else
    {
      gx[xi / BITS] &= ~(1U << (xi % BITS));
      gy[yi / BITS] &= ~(1U << (yi % BITS));
      gz[zi / BITS] &= ~(1U << (zi % BITS));
    }
  }

  unsigned int get(glm::ivec3 position)
  {
    return get(position.x, position.y, position.z);
  }

  void set(glm::ivec3 position, unsigned int value)
  {
    set(position.x, position.y, position.z, value);
  }

  const glm::ivec3 size()
  {
    return {SIZE, SIZE, SIZE};
  }

  uint32_t &getRow(unsigned int x, unsigned int y, unsigned int z)
  {
    return gx[(SIZE * (y + (SIZE * z))) / BITS];
  }

  uint32_t &getColumn(unsigned int x, unsigned int y, unsigned int z)
  {
    return gy[(SIZE * (x + (SIZE * z))) / BITS];
  }

  uint32_t &getLayer(unsigned int x, unsigned int y, unsigned int z)
  {
    return gz[(SIZE * (y + (SIZE * x))) / BITS];
  }

  void clear()
  {
    std::memset(gx, 0, sizeof(gx));
    std::memset(gy, 0, sizeof(gy));
    std::memset(gz, 0, sizeof(gz));
  }

  uint32_t createMask(unsigned int x)
  {
    if (x == 32)
      return 0U;
    return ~((1U << x) - 1);
  }
};