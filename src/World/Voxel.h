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
    static constexpr const uint8_t SIZE = sizeof(uint32_t) * 8;

  private:
    static constexpr const unsigned int MaskSize = ((Chunk::SIZE * Chunk::SIZE * Chunk::SIZE) / 8);

    std::vector<Voxel> grid{SIZE * SIZE * SIZE};

    /**
     * Create the columns/rows/layers mask
     * Extra 4kb for each of them in memory, however
     * during meshing, it saves ~30% execution time.
     */
    uint32_t rows[MaskSize] = {};
    uint32_t columns[MaskSize] = {};
    uint32_t layers[MaskSize] = {};

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

      const unsigned int yi = position.y + (SIZE * (position.x + (SIZE * position.z)));
      const unsigned int xi = position.x + (SIZE * (position.y + (SIZE * position.z)));
      const unsigned int zi = position.z + (SIZE * (position.y + (SIZE * position.x)));

      /**
       * Create the columns/rows/layers mask
       * Extra 4kb for each of them in memory, however
       * during meshing, it saves ~30% execution time.
       */
      if (type != Type::AIR)
      {
        columns[yi / SIZE] |= (1ULL << (yi % SIZE));
        rows[xi / SIZE] |= (1ULL << (xi % SIZE));
        layers[zi / SIZE] |= (1ULL << (zi % SIZE));
      }
      else
      {
        columns[yi / SIZE] &= ~(1ULL << (yi % SIZE));
        rows[xi / SIZE] &= ~(1ULL << (xi % SIZE));
        layers[zi / SIZE] &= ~(1ULL << (zi % SIZE));
      }
    }

    uint32_t *getRows()
    {
      return rows;
    }

    uint32_t *getColumns()
    {
      return columns;
    }

    uint32_t *getLayers()
    {
      return layers;
    }

    void clear()
    {
      grid.clear();
    }
  };

}
