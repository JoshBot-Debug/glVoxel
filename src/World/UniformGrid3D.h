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
    return y + (GRID_SIZE.y * (x + (GRID_SIZE.x * z)));
    // return x + (GRID_SIZE.x * (y + (GRID_SIZE.y * z)));
  }

  unsigned int getIndex(glm::ivec3 position) const
  {
    return getIndex(position.x, position.y, position.z);
  }

  glm::ivec3 getPosition(int index)
  {
    // int x = index % GRID_SIZE.x;
    // int y = (index / GRID_SIZE.x) % GRID_SIZE.y;
    // int z = (index / (GRID_SIZE.x * GRID_SIZE.y));

    int x = (index / GRID_SIZE.y) % GRID_SIZE.x;
    int y = index % GRID_SIZE.y;
    int z = (index / (GRID_SIZE.y * GRID_SIZE.x));

    std::cout << index << " | " << x << " " << y << " " << z << " " << std::endl;
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

  std::bitset<GRID_SIZE.y> getColumn(glm::ivec3 position = {1, 0, 0})
  {
    std::bitset<GRID_SIZE.y> bits(0);

    // setValue(0, 0);
    // setValue(1, 1);
    // setValue(2, 1);
    // setValue(3, 1);
    // setValue(4, 0);

    // setValue(5, 0);
    // setValue(6, 1);
    // setValue(7, 1);
    // setValue(8, 1);
    // setValue(9, 0);

    setValue({0, 0, 0}, 0);
    setValue({0, 1, 0}, 1);
    setValue({0, 2, 0}, 1);
    setValue({0, 3, 0}, 1);
    setValue({0, 4, 0}, 0);

    // setValue({1, 0, 0}, 0);
    // setValue({1, 1, 0}, 1);
    // setValue({1, 2, 0}, 1);
    // setValue({1, 3, 0}, 1);
    // setValue({1, 4, 0}, 0);

    glm::ivec3 pos = {0, 0, 0};

    const unsigned int columnIndex = getIndex(position);

    const unsigned int CHARS = (GRID_SIZE.x * GRID_SIZE.y * GRID_SIZE.z + BITS - 1) / BITS;

    for (size_t i = 0; i < CHARS; i++)
      std::cout << std::bitset<8>(grid[i]) << std::endl;

    // grid[columnIndex / BITS]
    // setValue(columnIndex, 0);

    // unsigned int mask = (1U << GRID_SIZE.y - position.y) - 1;

    // unsigned int result = (grid[columnIndex] >> position.y) & bits;

    // std::cout << "Result: " << std::bitset<32>(result) << std::endl;

    return bits;
    // std::bitset<GRID_SIZE.y> bits(0);

    // for (size_t i = position.y + center.y; i < bits.size(); i++)
    //   bits[i] = grid[getIndex(position.x, i, position.z)];

    // return bits;
  }
};