# glVoxel
- 21st December 2024

## Strat
- One VAO
- One VBO
- 9 IBO
- The IBO will have 9 chunks of 4096 * sizeof(Instance)
- These 9 IBOs will be reused to load different chunks as the player moves


#pragma once

#include <bit>
#include <bitset>
#include <glm/glm.hpp>

template <glm::ivec3 GRID_SIZE>
class UniformGrid3D
{
private:
  static constexpr unsigned int BITS = sizeof(char) * 8;

  char grid[(GRID_SIZE.x * GRID_SIZE.y * GRID_SIZE.z + BITS - 1) / BITS] = {};

public:
  unsigned int getIndex(int x, int y, int z) const
  {
    return x + (GRID_SIZE.x * (y + (GRID_SIZE.y * z)));
  }

  unsigned int getIndex(glm::ivec3 position) const
  {
    return getIndex(position.x, position.y, position.z);
  }

  glm::ivec3 getPosition(int index)
  {
    int x = index % GRID_SIZE.x;
    int y = (index / GRID_SIZE.x) % GRID_SIZE.y;
    int z = (index / (GRID_SIZE.x * GRID_SIZE.y));
    return {x, y, z};
  }

  unsigned int getValue(unsigned int index)
  {
    return (grid[index / BITS] >> index % BITS) & 1;
  }

  void setValue(unsigned int index, unsigned int value)
  {
    assert(value == 0 || value == 1);

    unsigned int arrayIndex = index / BITS;
    unsigned int bitIndex = index % BITS;

    if (value == 1)
      grid[arrayIndex] |= (1 << bitIndex);
    else
      grid[arrayIndex] &= ~(1 << bitIndex);
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

  const glm::ivec3 size() const
  {
    return GRID_SIZE;
  }

  const unsigned int count() const
  {
    return GRID_SIZE.x * GRID_SIZE.y * GRID_SIZE.z;
  }

  std::bitset<GRID_SIZE.y> getColumn(glm::ivec3 position)
  {
    std::bitset<GRID_SIZE.y> bits(0);

    setValue({0, 0, 0}, 0);
    setValue({0, 1, 0}, 0);
    setValue({0, 2, 0}, 0);
    setValue({0, 3, 0}, 0);
    setValue({0, 4, 0}, 0);

    glm::ivec3 pos = {0, 0, 0};

    const unsigned int columnIndex = pos.x + GRID_SIZE.x + (GRID_SIZE.y * pos.z);

    // std::cout << grid[columnIndex] << std::endl;

    std::cout << (grid[columnIndex] |= 1 << pos.y) << std::endl;

    return bits;
    // std::bitset<GRID_SIZE.y> bits(0);

    // for (size_t i = position.y + center.y; i < bits.size(); i++)
    //   bits[i] = grid[getIndex(position.x, i, position.z)];

    // return bits;
  }
};