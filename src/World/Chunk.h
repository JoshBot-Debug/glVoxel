#pragma once

#include <glm/glm.hpp>
#include <immintrin.h>
#include <string>
#include <vector>
#include <functional>

#include "Engine/Types.h"

struct IVec3Hash
{
  std::size_t operator()(const glm::ivec3 &v) const
  {
    return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1) ^ (std::hash<int>()(v.z) << 2);
  }
};

enum class FaceDirection
{
  TOP,
  BOTTOM,
  FRONT,
  BACK,
  LEFT,
  RIGHT
};

inline void generateFace(std::vector<Vertex> &vertices, float px, float py, float pz, float sx, float sy, float sz, const FaceDirection &direction)
{
  switch (direction)
  {
  case FaceDirection::TOP:
    vertices.emplace_back(Vertex{px, py + sy, pz, (int)direction});
    vertices.emplace_back(Vertex{px + sx, py + sy, pz + sz, (int)direction});
    vertices.emplace_back(Vertex{px + sx, py + sy, pz, (int)direction});

    vertices.emplace_back(Vertex{px, py + sy, pz, (int)direction});
    vertices.emplace_back(Vertex{px, py + sy, pz + sz, (int)direction});
    vertices.emplace_back(Vertex{px + sx, py + sy, pz + sz, (int)direction});
    break;
  case FaceDirection::BOTTOM:
    vertices.emplace_back(Vertex{px, py, pz, (int)direction});
    vertices.emplace_back(Vertex{px + sx, py, pz, (int)direction});
    vertices.emplace_back(Vertex{px + sx, py, pz + sz, (int)direction});

    vertices.emplace_back(Vertex{px, py, pz, (int)direction});
    vertices.emplace_back(Vertex{px + sx, py, pz + sz, (int)direction});
    vertices.emplace_back(Vertex{px, py, pz + sz, (int)direction});
    break;
  case FaceDirection::FRONT:
    vertices.emplace_back(Vertex{px, py, pz, (int)direction});
    vertices.emplace_back(Vertex{px + sx, py + sy, pz, (int)direction});
    vertices.emplace_back(Vertex{px + sx, py, pz, (int)direction});

    vertices.emplace_back(Vertex{px, py, pz, (int)direction});
    vertices.emplace_back(Vertex{px, py + sy, pz, (int)direction});
    vertices.emplace_back(Vertex{px + sx, py + sy, pz, (int)direction});
    break;
  case FaceDirection::BACK:
    vertices.emplace_back(Vertex{px, py, pz + sz, (int)direction});
    vertices.emplace_back(Vertex{px + sx, py, pz + sz, (int)direction});
    vertices.emplace_back(Vertex{px + sx, py + sy, pz + sz, (int)direction});

    vertices.emplace_back(Vertex{px, py, pz + sz, (int)direction});
    vertices.emplace_back(Vertex{px + sx, py + sy, pz + sz, (int)direction});
    vertices.emplace_back(Vertex{px, py + sy, pz + sz, (int)direction});
    break;
  case FaceDirection::LEFT:
    vertices.emplace_back(Vertex{px, py, pz, (int)direction});
    vertices.emplace_back(Vertex{px, py, pz + sz, (int)direction});
    vertices.emplace_back(Vertex{px, py + sy, pz + sz, (int)direction});

    vertices.emplace_back(Vertex{px, py, pz, (int)direction});
    vertices.emplace_back(Vertex{px, py + sy, pz + sz, (int)direction});
    vertices.emplace_back(Vertex{px, py + sy, pz, (int)direction});
    break;
  case FaceDirection::RIGHT:
    vertices.emplace_back(Vertex{px + sx, py, pz, (int)direction});
    vertices.emplace_back(Vertex{px + sx, py + sy, pz + sz, (int)direction});
    vertices.emplace_back(Vertex{px + sx, py, pz + sz, (int)direction});

    vertices.emplace_back(Vertex{px + sx, py, pz, (int)direction});
    vertices.emplace_back(Vertex{px + sx, py + sy, pz, (int)direction});
    vertices.emplace_back(Vertex{px + sx, py + sy, pz + sz, (int)direction});
    break;
  default:
    break;
  }
}

class Chunk
{
public:
  static constexpr const uint32_t SIZE = sizeof(uint32_t) * 8;
  static constexpr const uint32_t GRID_SIZE = (SIZE * SIZE * SIZE) / SIZE;

private:
  uint32_t gx[GRID_SIZE] = {};
  uint32_t gy[GRID_SIZE] = {};
  uint32_t gz[GRID_SIZE] = {};

  glm::ivec3 root = {0, 0, 0};

public:
  const unsigned int get(unsigned int x, unsigned int y, unsigned int z)
  {
    if (x < 0 || y < 0 || z < 0 || x >= SIZE || y >= SIZE || z >= SIZE)
      return 0;

    const unsigned int xi = x + (SIZE * (y + (SIZE * z)));

    return (gx[xi / SIZE] >> xi % SIZE) & 1;
  }

  void set(unsigned int x, unsigned int y, unsigned int z, unsigned int value)
  {
    assert(value == 0 || value == 1 && !(x < 0 || y < 0 || z < 0 || x >= SIZE || y >= SIZE || z >= SIZE));

    const unsigned int xi = x + (SIZE * (y + (SIZE * z)));
    const unsigned int yi = y + (SIZE * (x + (SIZE * z)));
    const unsigned int zi = z + (SIZE * (y + (SIZE * x)));

    if (value == 1)
    {
      gx[xi / SIZE] |= (1ULL << (xi % SIZE));
      gy[yi / SIZE] |= (1ULL << (yi % SIZE));
      gz[zi / SIZE] |= (1ULL << (zi % SIZE));
    }
    else
    {
      gx[xi / SIZE] &= ~(1ULL << (xi % SIZE));
      gy[yi / SIZE] &= ~(1ULL << (yi % SIZE));
      gz[zi / SIZE] &= ~(1ULL << (zi % SIZE));
    }
  }

  const unsigned int get(glm::ivec3 position)
  {
    return get(position.x, position.y, position.z);
  }

  void set(glm::ivec3 position, unsigned int value)
  {
    set(position.x, position.y, position.z, value);
  }

  void setRootCoordinate(glm::ivec3 coord)
  {
    root.x = SIZE * coord.x;
    root.y = SIZE * coord.y;
    root.z = SIZE * coord.z;
  }

  const glm::ivec3 size()
  {
    return {SIZE, SIZE, SIZE};
  }

  uint32_t &getRow(unsigned int x, unsigned int y, unsigned int z)
  {
    return gx[(SIZE * (y + (SIZE * z))) / SIZE];
  }

  uint32_t &getColumn(unsigned int x, unsigned int y, unsigned int z)
  {
    return gy[(SIZE * (x + (SIZE * z))) / SIZE];
  }

  uint32_t &getLayer(unsigned int x, unsigned int y, unsigned int z)
  {
    return gz[(SIZE * (y + (SIZE * x))) / SIZE];
  }

  void clear()
  {
    std::memset(gx, 0, sizeof(gx));
    std::memset(gy, 0, sizeof(gy));
    std::memset(gz, 0, sizeof(gz));
  }

  void mesh(std::vector<Vertex> &vertices)
  {
    uint32_t wfMask[Chunk::GRID_SIZE] = {};
    uint32_t hfMask[Chunk::GRID_SIZE] = {};

    uint32_t wlMask[Chunk::GRID_SIZE] = {};
    uint32_t hlMask[Chunk::GRID_SIZE] = {};

    for (size_t z = 0; z < Chunk::SIZE; z++)
    {
      for (size_t x = 0; x < Chunk::SIZE; x++)
      {
        const uint32_t column = this->getColumn(x, 0, z);
        uint32_t first = column & ~(column << 1);
        uint32_t last = column & ~(column >> 1);

        while (first)
        {
          const unsigned int w = __builtin_ffs(first) - 1;

          const unsigned int wi = x + (Chunk::SIZE * (w + (Chunk::SIZE * z)));
          wfMask[wi / Chunk::SIZE] |= (1ULL << (wi % Chunk::SIZE));

          const unsigned int hi = z + (Chunk::SIZE * (w + (Chunk::SIZE * x)));
          hfMask[hi / Chunk::SIZE] |= (1ULL << (hi % Chunk::SIZE));

          first &= ~((1ULL << w + 1) - 1);
        }

        while (last)
        {
          const unsigned int w = __builtin_ffs(last) - 1;

          const unsigned int wi = x + (Chunk::SIZE * (w + (Chunk::SIZE * z)));
          wlMask[wi / Chunk::SIZE] |= (1ULL << (wi % Chunk::SIZE));

          const unsigned int hi = z + (Chunk::SIZE * (w + (Chunk::SIZE * x)));
          hlMask[hi / Chunk::SIZE] |= (1ULL << (hi % Chunk::SIZE));

          last &= ~((1ULL << w + 1) - 1);
        }
      }
    }

    for (size_t z = 0; z < Chunk::SIZE; z++)
    {
      for (size_t x = 0; x < Chunk::SIZE; x++)
      {
        const uint32_t column = this->getColumn(x, 0, z);
        uint32_t first = column & ~(column << 1);
        uint32_t last = column & ~(column >> 1);

        while (first)
        {
          const unsigned int w = __builtin_ffs(first) - 1;
          first &= ~((1ULL << w + 1) - 1);

          uint32_t &width = wfMask[(Chunk::SIZE * (w + (Chunk::SIZE * z))) / Chunk::SIZE] &= ~((1ULL << x) - 1);

          if (!width)
            continue;

          const unsigned int wOffset = !width ? 0 : __builtin_ffs(width) - 1;
          const unsigned int wSize = __builtin_ctz(~(width >> (__builtin_ffs(width) - 1)));

          uint32_t &height = hfMask[(Chunk::SIZE * (w + (Chunk::SIZE * (int)(wOffset)))) / Chunk::SIZE] &= ~((1ULL << z) - 1);

          const unsigned int hOffset = !height ? 0 : __builtin_ffs(height) - 1;
          unsigned int hSize = __builtin_ctz(~(height >> (__builtin_ffs(height) - 1)));

          for (size_t i = hOffset; i < hOffset + hSize; i++)
          {
            const unsigned int index = (Chunk::SIZE * (w + (Chunk::SIZE * i))) / Chunk::SIZE;
            const uint32_t SIZE = wfMask[index] & (((1ULL << (int)wSize) - 1) << wOffset);

            if (__builtin_ctz(~(SIZE >> (__builtin_ffs(SIZE) - 1))) != wSize)
            {
              hSize = i - hOffset;
              break;
            }

            wfMask[index] &= ~(((1ULL << (int)wSize) - 1) << wOffset);
          }

          generateFace(vertices, wOffset + root.x, w + root.y, z + root.z, wSize, 1.0f, hSize, FaceDirection::BOTTOM);
        }

        while (last)
        {
          const unsigned int w = __builtin_ffs(last) - 1;
          last &= ~((1ULL << w + 1) - 1);

          uint32_t &width = wlMask[(Chunk::SIZE * (w + (Chunk::SIZE * z))) / Chunk::SIZE] &= ~((1ULL << x) - 1);

          if (!width)
            continue;

          const unsigned int wOffset = !width ? 0 : __builtin_ffs(width) - 1;
          const unsigned int wSize = __builtin_ctz(~(width >> (__builtin_ffs(width) - 1)));

          uint32_t &height = hlMask[(Chunk::SIZE * (w + (Chunk::SIZE * (int)(wOffset)))) / Chunk::SIZE] &= ~((1ULL << z) - 1);

          const unsigned int hOffset = !height ? 0 : __builtin_ffs(height) - 1;
          unsigned int hSize = __builtin_ctz(~(height >> (__builtin_ffs(height) - 1)));

          for (size_t i = hOffset; i < hOffset + hSize; i++)
          {
            const unsigned int index = (Chunk::SIZE * (w + (Chunk::SIZE * i))) / Chunk::SIZE;
            const uint32_t SIZE = wlMask[index] & (((1ULL << (int)wSize) - 1) << wOffset);

            if (__builtin_ctz(~(SIZE >> (__builtin_ffs(SIZE) - 1))) != wSize)
            {
              hSize = i - hOffset;
              break;
            }

            wlMask[index] &= ~(((1ULL << (int)wSize) - 1) << wOffset);
          }

          generateFace(vertices, wOffset + root.x, w + root.y, z + root.z, wSize, 1.0f, hSize, FaceDirection::TOP);
        }
      }
    }

    std::memset(wfMask, 0, sizeof(wfMask));
    std::memset(hfMask, 0, sizeof(hfMask));
    std::memset(wlMask, 0, sizeof(wlMask));
    std::memset(hlMask, 0, sizeof(hlMask));

    for (size_t z = 0; z < Chunk::SIZE; z++)
    {
      for (size_t y = 0; y < Chunk::SIZE; y++)
      {
        const uint32_t row = this->getRow(0, y, z);
        uint32_t first = row & ~(row << 1);
        uint32_t last = row & ~(row >> 1);

        while (first)
        {
          const unsigned int w = __builtin_ffs(first) - 1;

          const unsigned int wi = y + (Chunk::SIZE * (w + (Chunk::SIZE * z)));
          wfMask[wi / Chunk::SIZE] |= (1ULL << (wi % Chunk::SIZE));

          const unsigned int hi = z + (Chunk::SIZE * (w + (Chunk::SIZE * y)));
          hfMask[hi / Chunk::SIZE] |= (1ULL << (hi % Chunk::SIZE));

          first &= ~((1ULL << w + 1) - 1);
        }

        while (last)
        {
          const unsigned int w = __builtin_ffs(last) - 1;

          const unsigned int wi = y + (Chunk::SIZE * (w + (Chunk::SIZE * z)));
          wlMask[wi / Chunk::SIZE] |= (1ULL << (wi % Chunk::SIZE));

          const unsigned int hi = z + (Chunk::SIZE * (w + (Chunk::SIZE * y)));
          hlMask[hi / Chunk::SIZE] |= (1ULL << (hi % Chunk::SIZE));

          last &= ~((1ULL << w + 1) - 1);
        }
      }
    }

    for (size_t z = 0; z < Chunk::SIZE; z++)
    {
      for (size_t y = 0; y < Chunk::SIZE; y++)
      {
        const uint32_t row = this->getRow(0, y, z);
        uint32_t first = row & ~(row << 1);
        uint32_t last = row & ~(row >> 1);

        while (first)
        {
          const unsigned int w = __builtin_ffs(first) - 1;
          first &= ~((1ULL << w + 1) - 1);

          uint32_t &width = wfMask[(Chunk::SIZE * (w + (Chunk::SIZE * z))) / Chunk::SIZE] &= ~((1ULL << y) - 1);

          if (!width)
            continue;

          const unsigned int wOffset = !width ? 0 : __builtin_ffs(width) - 1;
          const unsigned int wSize = __builtin_ctz(~(width >> (__builtin_ffs(width) - 1)));

          uint32_t &height = hfMask[(Chunk::SIZE * (w + (Chunk::SIZE * (int)(wOffset)))) / Chunk::SIZE] &= ~((1ULL << z) - 1);

          const unsigned int hOffset = !height ? 0 : __builtin_ffs(height) - 1;
          unsigned int hSize = __builtin_ctz(~(height >> (__builtin_ffs(height) - 1)));

          for (size_t i = hOffset; i < hOffset + hSize; i++)
          {
            const unsigned int index = (Chunk::SIZE * (w + (Chunk::SIZE * i))) / Chunk::SIZE;
            const uint32_t SIZE = wfMask[index] & (((1ULL << (int)wSize) - 1) << wOffset);

            if (__builtin_ctz(~(SIZE >> (__builtin_ffs(SIZE) - 1))) != wSize)
            {
              hSize = i - hOffset;
              break;
            }

            wfMask[index] &= ~(((1ULL << (int)wSize) - 1) << wOffset);
          }

          generateFace(vertices, w + root.x, wOffset + root.y, z + root.z, 1.0f, wSize, hSize, FaceDirection::LEFT);
        }

        while (last)
        {
          const unsigned int w = __builtin_ffs(last) - 1;
          last &= ~((1ULL << w + 1) - 1);

          uint32_t &width = wlMask[(Chunk::SIZE * (w + (Chunk::SIZE * z))) / Chunk::SIZE] &= ~((1ULL << y) - 1);

          if (!width)
            continue;

          const unsigned int wOffset = !width ? 0 : __builtin_ffs(width) - 1;
          const unsigned int wSize = __builtin_ctz(~(width >> (__builtin_ffs(width) - 1)));

          uint32_t &height = hlMask[(Chunk::SIZE * (w + (Chunk::SIZE * (int)(wOffset)))) / Chunk::SIZE] &= ~((1ULL << z) - 1);

          const unsigned int hOffset = !height ? 0 : __builtin_ffs(height) - 1;
          unsigned int hSize = __builtin_ctz(~(height >> (__builtin_ffs(height) - 1)));

          for (size_t i = hOffset; i < hOffset + hSize; i++)
          {
            const unsigned int index = (Chunk::SIZE * (w + (Chunk::SIZE * i))) / Chunk::SIZE;
            const uint32_t SIZE = wlMask[index] & (((1ULL << (int)wSize) - 1) << wOffset);

            if (__builtin_ctz(~(SIZE >> (__builtin_ffs(SIZE) - 1))) != wSize)
            {
              hSize = i - hOffset;
              break;
            }

            wlMask[index] &= ~(((1ULL << (int)wSize) - 1) << wOffset);
          }

          generateFace(vertices, w + root.x, wOffset + root.y, z + root.z, 1.0f, wSize, hSize, FaceDirection::RIGHT);
        }
      }
    }

    std::memset(wfMask, 0, sizeof(wfMask));
    std::memset(hfMask, 0, sizeof(hfMask));
    std::memset(wlMask, 0, sizeof(wlMask));
    std::memset(hlMask, 0, sizeof(hlMask));

    for (size_t x = 0; x < Chunk::SIZE; x++)
    {
      for (size_t y = 0; y < Chunk::SIZE; y++)
      {
        const uint32_t depth = this->getLayer(x, y, 0);
        uint32_t first = depth & ~(depth << 1);
        uint32_t last = depth & ~(depth >> 1);

        while (first)
        {
          const unsigned int w = __builtin_ffs(first) - 1;

          const unsigned int wi = y + (Chunk::SIZE * (w + (Chunk::SIZE * x)));
          wfMask[wi / Chunk::SIZE] |= (1ULL << (wi % Chunk::SIZE));

          const unsigned int hi = x + (Chunk::SIZE * (w + (Chunk::SIZE * y)));
          hfMask[hi / Chunk::SIZE] |= (1ULL << (hi % Chunk::SIZE));

          first &= ~((1ULL << w + 1) - 1);
        }

        while (last)
        {
          const unsigned int w = __builtin_ffs(last) - 1;

          const unsigned int wi = y + (Chunk::SIZE * (w + (Chunk::SIZE * x)));
          wlMask[wi / Chunk::SIZE] |= (1ULL << (wi % Chunk::SIZE));

          const unsigned int hi = x + (Chunk::SIZE * (w + (Chunk::SIZE * y)));
          hlMask[hi / Chunk::SIZE] |= (1ULL << (hi % Chunk::SIZE));

          last &= ~((1ULL << w + 1) - 1);
        }
      }
    }

    for (size_t x = 0; x < Chunk::SIZE; x++)
    {
      for (size_t y = 0; y < Chunk::SIZE; y++)
      {
        const uint32_t depth = this->getLayer(x, y, 0);
        uint32_t first = depth & ~(depth << 1);
        uint32_t last = depth & ~(depth >> 1);

        while (first)
        {
          const unsigned int w = __builtin_ffs(first) - 1;
          first &= ~((1ULL << w + 1) - 1);

          uint32_t &width = wfMask[(Chunk::SIZE * (w + (Chunk::SIZE * x))) / Chunk::SIZE] &= ~((1ULL << y) - 1);

          if (!width)
            continue;

          const unsigned int wOffset = !width ? 0 : __builtin_ffs(width) - 1;
          const unsigned int wSize = __builtin_ctz(~(width >> (__builtin_ffs(width) - 1)));

          uint32_t &height = hfMask[(Chunk::SIZE * (w + (Chunk::SIZE * (int)(wOffset)))) / Chunk::SIZE] &= ~((1ULL << x) - 1);

          const unsigned int hOffset = !height ? 0 : __builtin_ffs(height) - 1;
          unsigned int hSize = __builtin_ctz(~(height >> (__builtin_ffs(height) - 1)));

          for (size_t i = hOffset; i < hOffset + hSize; i++)
          {
            const unsigned int index = (Chunk::SIZE * (w + (Chunk::SIZE * i))) / Chunk::SIZE;
            const uint32_t SIZE = wfMask[index] & (((1ULL << (int)wSize) - 1) << wOffset);

            if (__builtin_ctz(~(SIZE >> (__builtin_ffs(SIZE) - 1))) != wSize)
            {
              hSize = i - hOffset;
              break;
            }

            wfMask[index] &= ~(((1ULL << (int)wSize) - 1) << wOffset);
          }

          generateFace(vertices, x + root.x, wOffset + root.y, w + root.z, hSize, wSize, 1.0f, FaceDirection::FRONT);
        }

        while (last)
        {
          const unsigned int w = __builtin_ffs(last) - 1;
          last &= ~((1ULL << w + 1) - 1);

          uint32_t &width = wlMask[(Chunk::SIZE * (w + (Chunk::SIZE * x))) / Chunk::SIZE] &= ~((1ULL << y) - 1);

          if (!width)
            continue;

          const unsigned int wOffset = !width ? 0 : __builtin_ffs(width) - 1;
          const unsigned int wSize = __builtin_ctz(~(width >> (__builtin_ffs(width) - 1)));

          uint32_t &height = hlMask[(Chunk::SIZE * (w + (Chunk::SIZE * (int)(wOffset)))) / Chunk::SIZE] &= ~((1ULL << x) - 1);

          const unsigned int hOffset = !height ? 0 : __builtin_ffs(height) - 1;
          unsigned int hSize = __builtin_ctz(~(height >> (__builtin_ffs(height) - 1)));

          for (size_t i = hOffset; i < hOffset + hSize; i++)
          {
            const unsigned int index = (Chunk::SIZE * (w + (Chunk::SIZE * i))) / Chunk::SIZE;
            const uint32_t SIZE = wlMask[index] & (((1ULL << (int)wSize) - 1) << wOffset);

            if (__builtin_ctz(~(SIZE >> (__builtin_ffs(SIZE) - 1))) != wSize)
            {
              hSize = i - hOffset;
              break;
            }

            wlMask[index] &= ~(((1ULL << (int)wSize) - 1) << wOffset);
          }

          generateFace(vertices, x + root.x, wOffset + root.y, w + root.z, hSize, wSize, 1.0f, FaceDirection::BACK);
        }
      }
    }
  }
};