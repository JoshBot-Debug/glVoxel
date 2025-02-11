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
    static constexpr const uint8_t ChunkSize = sizeof(uint32_t) * 8;
    static constexpr const unsigned int MaskLength = ((ChunkSize * ChunkSize * ChunkSize) / ChunkSize);

  private:
    std::vector<Voxel> grid{ChunkSize * ChunkSize * ChunkSize};

    unsigned int index(const glm::ivec3 &position)
    {
      return position.x + (ChunkSize * (position.y + (ChunkSize * position.z)));
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
