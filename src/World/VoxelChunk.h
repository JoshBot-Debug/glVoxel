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

class VoxelChunk
{
public:
  static constexpr const uint32_t SIZE = 32;
  static constexpr const uint32_t BITS = sizeof(uint32_t) * 8;
  static constexpr const uint32_t GRID_SIZE = (SIZE * SIZE * SIZE) / BITS;

private:
  uint32_t gx[GRID_SIZE] = {};
  uint32_t gy[GRID_SIZE] = {};
  uint32_t gz[GRID_SIZE] = {};

public:
  const unsigned int get(int x, int y, int z)
  {
    if (x < 0 || y < 0 || z < 0 || x >= SIZE || y >= SIZE || z >= SIZE)
      return 0;

    const unsigned int xi = x + (SIZE * (y + (SIZE * z)));

    return (gx[xi / BITS] >> xi % BITS) & 1;
  }

  void set(int x, int y, int z, unsigned int value)
  {
    assert(value == 0 || value == 1 && !(x < 0 || y < 0 || z < 0 || x >= SIZE || y >= SIZE || z >= SIZE));

    const unsigned int xi = x + (SIZE * (y + (SIZE * z)));
    const unsigned int yi = y + (SIZE * (x + (SIZE * z)));
    const unsigned int zi = z + (SIZE * (y + (SIZE * x)));

    if (value == 1)
    {
      gx[xi / BITS] |= (1ULL << (xi % BITS));
      gy[yi / BITS] |= (1ULL << (yi % BITS));
      gz[zi / BITS] |= (1ULL << (zi % BITS));
    }
    else
    {
      gx[xi / BITS] &= ~(1ULL << (xi % BITS));
      gy[yi / BITS] &= ~(1ULL << (yi % BITS));
      gz[zi / BITS] &= ~(1ULL << (zi % BITS));
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

  void mesh(std::vector<Vertex> &vertices)
  {
    uint32_t wfMask[VoxelChunk::GRID_SIZE] = {};
    uint32_t hfMask[VoxelChunk::GRID_SIZE] = {};

    uint32_t wlMask[VoxelChunk::GRID_SIZE] = {};
    uint32_t hlMask[VoxelChunk::GRID_SIZE] = {};

    for (size_t z = 0; z < VoxelChunk::SIZE; z++)
    {
      for (size_t x = 0; x < VoxelChunk::SIZE; x++)
      {
        uint32_t &column = this->getColumn(x, 0, z);
        uint32_t first = column & ~(column << 1);
        uint32_t last = column & ~(column >> 1);

        while (first)
        {
          const unsigned int w = __builtin_ffs(first) - 1;

          const unsigned int wi = x + (VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * z)));
          wfMask[wi / VoxelChunk::BITS] |= (1ULL << (wi % VoxelChunk::BITS));

          const unsigned int hi = z + (VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * x)));
          hfMask[hi / VoxelChunk::BITS] |= (1ULL << (hi % VoxelChunk::BITS));

          /**
           * This bit shift works because we are use a 32bit int to hold out bits
           * and we are shifting into a 64bit integer. If we try to use a 64bit int to hold our
           * voxels, we'll need to make sure w+1 is less than 64. If it's 64, we should directly set
           * first to 0ULL
           */
          first &= ~((1ULL << w + 1) - 1);
        }

        while (last)
        {
          const unsigned int w = __builtin_ffs(last) - 1;

          const unsigned int wi = x + (VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * z)));
          wlMask[wi / VoxelChunk::BITS] |= (1ULL << (wi % VoxelChunk::BITS));

          const unsigned int hi = z + (VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * x)));
          hlMask[hi / VoxelChunk::BITS] |= (1ULL << (hi % VoxelChunk::BITS));

          last &= ~((1ULL << w + 1) - 1);
        }
      }
    }

    for (size_t z = 0; z < VoxelChunk::SIZE; z++)
    {
      for (size_t x = 0; x < VoxelChunk::SIZE; x++)
      {
        uint32_t &column = this->getColumn(x, 0, z);
        uint32_t first = column & ~(column << 1);
        uint32_t last = column & ~(column >> 1);

        while (first)
        {
          const unsigned int w = __builtin_ffs(first) - 1;
          first &= ~((1ULL << w + 1) - 1);

          uint32_t &width = wfMask[(VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * z))) / VoxelChunk::BITS] &= ~((1ULL << x) - 1);

          if (!width)
            continue;

          const unsigned int wOffset = !width ? 0 : __builtin_ffs(width) - 1;
          const unsigned int wSize = __builtin_ctz(~(width >> (__builtin_ffs(width) - 1)));

          uint32_t &height = hfMask[(VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * (int)(wOffset)))) / VoxelChunk::BITS] &= ~((1ULL << z) - 1);

          const unsigned int hOffset = !height ? 0 : __builtin_ffs(height) - 1;
          unsigned int hSize = __builtin_ctz(~(height >> (__builtin_ffs(height) - 1)));

          for (size_t i = hOffset; i < hOffset + hSize; i++)
          {
            const unsigned int index = (VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * i))) / VoxelChunk::BITS;
            const uint32_t bits = wfMask[index] & (((1ULL << (int)wSize) - 1) << wOffset);

            if (__builtin_ctz(~(bits >> (__builtin_ffs(bits) - 1))) != wSize)
            {
              hSize = i - hOffset;
              break;
            }

            wfMask[index] &= ~(((1ULL << (int)wSize) - 1) << wOffset);
          }

          generateFace(vertices, wOffset, w, z, wSize, 1.0f, hSize, FaceDirection::BOTTOM);
        }

        while (last)
        {
          const unsigned int w = __builtin_ffs(last) - 1;
          last &= ~((1ULL << w + 1) - 1);

          uint32_t &width = wlMask[(VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * z))) / VoxelChunk::BITS] &= ~((1ULL << x) - 1);

          if (!width)
            continue;

          const unsigned int wOffset = !width ? 0 : __builtin_ffs(width) - 1;
          const unsigned int wSize = __builtin_ctz(~(width >> (__builtin_ffs(width) - 1)));

          uint32_t &height = hlMask[(VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * (int)(wOffset)))) / VoxelChunk::BITS] &= ~((1ULL << z) - 1);

          const unsigned int hOffset = !height ? 0 : __builtin_ffs(height) - 1;
          unsigned int hSize = __builtin_ctz(~(height >> (__builtin_ffs(height) - 1)));

          for (size_t i = hOffset; i < hOffset + hSize; i++)
          {
            const unsigned int index = (VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * i))) / VoxelChunk::BITS;
            const uint32_t bits = wlMask[index] & (((1ULL << (int)wSize) - 1) << wOffset);

            if (__builtin_ctz(~(bits >> (__builtin_ffs(bits) - 1))) != wSize)
            {
              hSize = i - hOffset;
              break;
            }

            wlMask[index] &= ~(((1ULL << (int)wSize) - 1) << wOffset);
          }

          generateFace(vertices, wOffset, w, z, wSize, 1.0f, hSize, FaceDirection::TOP);
        }
      }
    }

    std::memset(wfMask, 0, sizeof(wfMask));
    std::memset(hfMask, 0, sizeof(hfMask));
    std::memset(wlMask, 0, sizeof(wlMask));
    std::memset(hlMask, 0, sizeof(hlMask));

    for (size_t z = 0; z < VoxelChunk::SIZE; z++)
    {
      for (size_t y = 0; y < VoxelChunk::SIZE; y++)
      {
        uint32_t &row = this->getRow(0, y, z);
        uint32_t first = row & ~(row << 1);
        uint32_t last = row & ~(row >> 1);

        while (first)
        {
          const unsigned int w = __builtin_ffs(first) - 1;

          const unsigned int wi = y + (VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * z)));
          wfMask[wi / VoxelChunk::BITS] |= (1ULL << (wi % VoxelChunk::BITS));

          const unsigned int hi = z + (VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * y)));
          hfMask[hi / VoxelChunk::BITS] |= (1ULL << (hi % VoxelChunk::BITS));

          first &= ~((1ULL << w + 1) - 1);
        }

        while (last)
        {
          const unsigned int w = __builtin_ffs(last) - 1;

          const unsigned int wi = y + (VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * z)));
          wlMask[wi / VoxelChunk::BITS] |= (1ULL << (wi % VoxelChunk::BITS));

          const unsigned int hi = z + (VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * y)));
          hlMask[hi / VoxelChunk::BITS] |= (1ULL << (hi % VoxelChunk::BITS));

          last &= ~((1ULL << w + 1) - 1);
        }
      }
    }

    for (size_t z = 0; z < VoxelChunk::SIZE; z++)
    {
      for (size_t y = 0; y < VoxelChunk::SIZE; y++)
      {
        uint32_t &row = this->getRow(0, y, z);
        uint32_t first = row & ~(row << 1);
        uint32_t last = row & ~(row >> 1);

        while (first)
        {
          const unsigned int w = __builtin_ffs(first) - 1;
          first &= ~((1ULL << w + 1) - 1);

          uint32_t &width = wfMask[(VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * z))) / VoxelChunk::BITS] &= ~((1ULL << y) - 1);

          if (!width)
            continue;

          const unsigned int wOffset = !width ? 0 : __builtin_ffs(width) - 1;
          const unsigned int wSize = __builtin_ctz(~(width >> (__builtin_ffs(width) - 1)));

          uint32_t &height = hfMask[(VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * (int)(wOffset)))) / VoxelChunk::BITS] &= ~((1ULL << z) - 1);

          const unsigned int hOffset = !height ? 0 : __builtin_ffs(height) - 1;
          unsigned int hSize = __builtin_ctz(~(height >> (__builtin_ffs(height) - 1)));

          for (size_t i = hOffset; i < hOffset + hSize; i++)
          {
            const unsigned int index = (VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * i))) / VoxelChunk::BITS;
            const uint32_t bits = wfMask[index] & (((1ULL << (int)wSize) - 1) << wOffset);

            if (__builtin_ctz(~(bits >> (__builtin_ffs(bits) - 1))) != wSize)
            {
              hSize = i - hOffset;
              break;
            }

            wfMask[index] &= ~(((1ULL << (int)wSize) - 1) << wOffset);
          }

          generateFace(vertices, w, wOffset, z, 1.0f, wSize, hSize, FaceDirection::LEFT);
        }

        while (last)
        {
          const unsigned int w = __builtin_ffs(last) - 1;
          last &= ~((1ULL << w + 1) - 1);

          uint32_t &width = wlMask[(VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * z))) / VoxelChunk::BITS] &= ~((1ULL << y) - 1);

          if (!width)
            continue;

          const unsigned int wOffset = !width ? 0 : __builtin_ffs(width) - 1;
          const unsigned int wSize = __builtin_ctz(~(width >> (__builtin_ffs(width) - 1)));

          uint32_t &height = hlMask[(VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * (int)(wOffset)))) / VoxelChunk::BITS] &= ~((1ULL << z) - 1);

          const unsigned int hOffset = !height ? 0 : __builtin_ffs(height) - 1;
          unsigned int hSize = __builtin_ctz(~(height >> (__builtin_ffs(height) - 1)));

          for (size_t i = hOffset; i < hOffset + hSize; i++)
          {
            const unsigned int index = (VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * i))) / VoxelChunk::BITS;
            const uint32_t bits = wlMask[index] & (((1ULL << (int)wSize) - 1) << wOffset);

            if (__builtin_ctz(~(bits >> (__builtin_ffs(bits) - 1))) != wSize)
            {
              hSize = i - hOffset;
              break;
            }

            wlMask[index] &= ~(((1ULL << (int)wSize) - 1) << wOffset);
          }

          generateFace(vertices, w, wOffset, z, 1.0f, wSize, hSize, FaceDirection::RIGHT);
        }
      }
    }

    std::memset(wfMask, 0, sizeof(wfMask));
    std::memset(hfMask, 0, sizeof(hfMask));
    std::memset(wlMask, 0, sizeof(wlMask));
    std::memset(hlMask, 0, sizeof(hlMask));

    for (size_t x = 0; x < VoxelChunk::SIZE; x++)
    {
      for (size_t y = 0; y < VoxelChunk::SIZE; y++)
      {
        uint32_t &depth = this->getLayer(x, y, 0);
        uint32_t first = depth & ~(depth << 1);
        uint32_t last = depth & ~(depth >> 1);

        while (first)
        {
          const unsigned int w = __builtin_ffs(first) - 1;

          const unsigned int wi = y + (VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * x)));
          wfMask[wi / VoxelChunk::BITS] |= (1ULL << (wi % VoxelChunk::BITS));

          const unsigned int hi = x + (VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * y)));
          hfMask[hi / VoxelChunk::BITS] |= (1ULL << (hi % VoxelChunk::BITS));

          first &= ~((1ULL << w + 1) - 1);
        }

        while (last)
        {
          const unsigned int w = __builtin_ffs(last) - 1;

          const unsigned int wi = y + (VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * x)));
          wlMask[wi / VoxelChunk::BITS] |= (1ULL << (wi % VoxelChunk::BITS));

          const unsigned int hi = x + (VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * y)));
          hlMask[hi / VoxelChunk::BITS] |= (1ULL << (hi % VoxelChunk::BITS));

          last &= ~((1ULL << w + 1) - 1);
        }
      }
    }

    for (size_t x = 0; x < VoxelChunk::SIZE; x++)
    {
      for (size_t y = 0; y < VoxelChunk::SIZE; y++)
      {
        uint32_t &depth = this->getLayer(x, y, 0);
        uint32_t first = depth & ~(depth << 1);
        uint32_t last = depth & ~(depth >> 1);

        while (first)
        {
          const unsigned int w = __builtin_ffs(first) - 1;
          first &= ~((1ULL << w + 1) - 1);

          uint32_t &width = wfMask[(VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * x))) / VoxelChunk::BITS] &= ~((1ULL << y) - 1);

          if (!width)
            continue;

          const unsigned int wOffset = !width ? 0 : __builtin_ffs(width) - 1;
          const unsigned int wSize = __builtin_ctz(~(width >> (__builtin_ffs(width) - 1)));

          uint32_t &height = hfMask[(VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * (int)(wOffset)))) / VoxelChunk::BITS] &= ~((1ULL << x) - 1);

          const unsigned int hOffset = !height ? 0 : __builtin_ffs(height) - 1;
          unsigned int hSize = __builtin_ctz(~(height >> (__builtin_ffs(height) - 1)));

          for (size_t i = hOffset; i < hOffset + hSize; i++)
          {
            const unsigned int index = (VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * i))) / VoxelChunk::BITS;
            const uint32_t bits = wfMask[index] & (((1ULL << (int)wSize) - 1) << wOffset);

            if (__builtin_ctz(~(bits >> (__builtin_ffs(bits) - 1))) != wSize)
            {
              hSize = i - hOffset;
              break;
            }

            wfMask[index] &= ~(((1ULL << (int)wSize) - 1) << wOffset);
          }

          generateFace(vertices, x, wOffset, w, hSize, wSize, 1.0f, FaceDirection::FRONT);
        }

        while (last)
        {
          const unsigned int w = __builtin_ffs(last) - 1;
          last &= ~((1ULL << w + 1) - 1);

          uint32_t &width = wlMask[(VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * x))) / VoxelChunk::BITS] &= ~((1ULL << y) - 1);

          if (!width)
            continue;

          const unsigned int wOffset = !width ? 0 : __builtin_ffs(width) - 1;
          const unsigned int wSize = __builtin_ctz(~(width >> (__builtin_ffs(width) - 1)));

          uint32_t &height = hlMask[(VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * (int)(wOffset)))) / VoxelChunk::BITS] &= ~((1ULL << x) - 1);

          const unsigned int hOffset = !height ? 0 : __builtin_ffs(height) - 1;
          unsigned int hSize = __builtin_ctz(~(height >> (__builtin_ffs(height) - 1)));

          for (size_t i = hOffset; i < hOffset + hSize; i++)
          {
            const unsigned int index = (VoxelChunk::SIZE * (w + (VoxelChunk::SIZE * i))) / VoxelChunk::BITS;
            const uint32_t bits = wlMask[index] & (((1ULL << (int)wSize) - 1) << wOffset);

            if (__builtin_ctz(~(bits >> (__builtin_ffs(bits) - 1))) != wSize)
            {
              hSize = i - hOffset;
              break;
            }

            wlMask[index] &= ~(((1ULL << (int)wSize) - 1) << wOffset);
          }

          generateFace(vertices, x, wOffset, w, hSize, wSize, 1.0f, FaceDirection::BACK);
        }
      }
    }
  }
};