#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include "Utility.h"

namespace Voxel
{
  enum class Type : unsigned char
  {
    AIR,
    GRASS,
    DIRT
  };

  struct Voxel
  {
    Type type = Type::AIR;

    const bool isSolid() const { return type != Type::AIR; };
  };

  class Chunk
  {
  public:
    static constexpr const unsigned int SIZE = sizeof(uint32_t) * 8;

  private:
    std::vector<Voxel> grid{SIZE * SIZE * SIZE};

    unsigned int index(const glm::ivec3 &position)
    {
      return position.x + (SIZE * (position.y + (SIZE * position.z)));
    }

  public:
    Voxel &get(const glm::ivec3 &position)
    {
      return grid[index(position)];
    }

    void set(const glm::ivec3 &position, const Type &type)
    {
      grid[index(position)].type = type;
    }

    void clear()
    {
      grid.clear();
    }
  };

}
