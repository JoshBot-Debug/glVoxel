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
#if __AVX2__
  alignas(32) uint32_t gridBT[GRID_SIZE] = {};
  alignas(32) uint32_t gridLR[GRID_SIZE] = {};
  alignas(32) uint32_t gridFB[GRID_SIZE] = {};
#else
  uint32_t gridBT[GRID_SIZE] = {};
  uint32_t gridLR[GRID_SIZE] = {};
  uint32_t gridFB[GRID_SIZE] = {};
#endif

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
    unsigned int fbIndex = z + (SIZE * (y + (SIZE * x)));

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
    return gridBT[(SIZE * (x + (SIZE * z))) / BITS];
  }

  void setColumn(unsigned int x, unsigned int y, unsigned int z, uint32_t value)
  {
    gridBT[(SIZE * (x + (SIZE * z))) / BITS] = value;
  }

  uint32_t &getRow(unsigned int x, unsigned int y, unsigned int z)
  {
    return gridLR[(SIZE * (y + (SIZE * z))) / BITS];
  }

  void setRow(unsigned int x, unsigned int y, unsigned int z, uint32_t value)
  {
    gridLR[(SIZE * (y + (SIZE * z))) / BITS] = value;
  }

  uint32_t &getDepth(unsigned int x, unsigned int y, unsigned int z)
  {
    return gridFB[(SIZE * (y + (SIZE * x))) / BITS];
  }

  void setDepth(unsigned int x, unsigned int y, unsigned int z, uint32_t value)
  {
    gridFB[(SIZE * (y + (SIZE * x))) / BITS] = value;
  }

  void clear()
  {
    std::memset(gridBT, 0, sizeof(gridBT));
    std::memset(gridLR, 0, sizeof(gridLR));
    std::memset(gridFB, 0, sizeof(gridFB));
  }

  void applyRowMask(uint32_t *data, uint32_t mask, int x, int y, int z, unsigned int count)
  {
    // unsigned int endIndex = startIndex + count;
    // for (size_t z = 0; z < size.z; z++)
    // {
    //   for (size_t x = 0; x < size.x; x++)
    //   {
    //   }
    // }

    for (unsigned int i = 0; i < count; ++i)
    {
      data[(SIZE * ((y + i) + (SIZE * (z)))) / BITS] &= mask;
    }
  }

  void applyMask(uint32_t *data, uint32_t mask, unsigned int startIndex, unsigned int count)
  {
#if __AVX2__
    __m256i vec_mask = _mm256_set1_epi32(mask);

    unsigned int endIndex = startIndex + count;

    assert(data != nullptr);
    assert(endIndex > startIndex);

    // Process the entire array in chunks of 8
    for (size_t i = startIndex; i < endIndex; i += 8)
    {
      unsigned int remaining = endIndex - i;

      if (remaining < 8)
      {
        for (unsigned int j = 0; j < remaining; ++j)
          data[i + j] &= mask;
        break;
      }

      // Load 8 elements into the SIMD register
      __m256i vec_data = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(&data[i]));
      // Apply the mask
      vec_data = _mm256_and_si256(vec_data, vec_mask);
      // Store the results back into the array
      _mm256_storeu_si256(reinterpret_cast<__m256i *>(&data[i]), vec_data);
    }
#else
    // fallback to scalar implementation
    unsigned int endIndex = startIndex + count;
    for (unsigned int i = startIndex; i < endIndex; ++i)
    {
      data[i] &= mask;
    }
#endif
  }

  uint32_t createMask(unsigned int x)
  {
    if (x == 32)
      return 0U;
    return ~((1U << x) - 1);
  }
};