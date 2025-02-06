#pragma once

#include <vector>
#include <bitset>

#include <glm/glm.hpp>

#include "Voxel.h"
#include "Engine/Types.h"
#include "Engine/Face.h"

class GreedyMesh
{
private:
  static void SetWidthHeight(unsigned int a, unsigned int b, uint32_t bits, uint32_t (&widthMasks)[], uint32_t (&heightMasks)[])
  {
    while (bits)
    {
      const unsigned int w = __builtin_ffs(bits) - 1;

      const unsigned int wi = a + (Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * b)));
      widthMasks[wi / Voxel::Chunk::SIZE] |= (1ULL << (wi % Voxel::Chunk::SIZE));

      const unsigned int hi = b + (Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * a)));
      heightMasks[hi / Voxel::Chunk::SIZE] |= (1ULL << (hi % Voxel::Chunk::SIZE));

      bits &= ~((1ULL << w + 1) - 1);
    }
  }

  static void PrepareWidthHeighMasks(const uint32_t (&bits)[], uint32_t (&widthStart)[], uint32_t (&heightStart)[], uint32_t (&widthEnd)[], uint32_t (&heightEnd)[])
  {
    for (size_t a = 0; a < Voxel::Chunk::SIZE; a++)
    {
      for (size_t b = 0; b < Voxel::Chunk::SIZE; b++)
      {
        const uint32_t mask = bits[(Voxel::Chunk::SIZE * (a + (Voxel::Chunk::SIZE * b))) / Voxel::Chunk::SIZE];
        SetWidthHeight(a, b, mask & ~(mask << 1), widthStart, heightStart);
        SetWidthHeight(a, b, mask & ~(mask >> 1), widthEnd, heightEnd);
      }
    }
  }

  static void GreedyMeshFace(const glm::ivec3 &offsetPosition, int a, int b, uint32_t bits, uint32_t (&widthMasks)[], uint32_t (&heightMasks)[], std::vector<Vertex> &vertices, FaceType type)
  {
    while (bits)
    {
      const unsigned int w = __builtin_ffs(bits) - 1;
      bits &= ~((1ULL << w + 1) - 1);

      uint32_t &width = widthMasks[(Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * a))) / Voxel::Chunk::SIZE] &= ~((1ULL << b) - 1);

      if (!width)
        continue;

      const unsigned int widthOffset = !width ? 0 : __builtin_ffs(width) - 1;
      const unsigned int widthSize = __builtin_ctz(~(width >> (__builtin_ffs(width) - 1)));

      uint32_t &height = heightMasks[(Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * (int)(widthOffset)))) / Voxel::Chunk::SIZE] &= ~((1ULL << a) - 1);

      const unsigned int heightOffset = !height ? 0 : __builtin_ffs(height) - 1;
      unsigned int heightSize = __builtin_ctz(~(height >> (__builtin_ffs(height) - 1)));

      for (size_t i = heightOffset; i < heightOffset + heightSize; i++)
      {
        const unsigned int index = (Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * i))) / Voxel::Chunk::SIZE;
        const uint32_t SIZE = widthMasks[index] & (((1ULL << (int)widthSize) - 1) << widthOffset);

        if (__builtin_ctz(~(SIZE >> (__builtin_ffs(SIZE) - 1))) != widthSize)
        {
          heightSize = i - heightOffset;
          break;
        }

        widthMasks[index] &= ~(((1ULL << (int)widthSize) - 1) << widthOffset);
      }

      switch (type)
      {
      case FaceType::TOP:
        Face::Top(vertices, widthOffset + (offsetPosition.x * Voxel::Chunk::SIZE), w + (offsetPosition.y * Voxel::Chunk::SIZE), a + (offsetPosition.z * Voxel::Chunk::SIZE), widthSize, 1.0f, heightSize);
        break;
      case FaceType::BOTTOM:
        Face::Bottom(vertices, widthOffset + (offsetPosition.x * Voxel::Chunk::SIZE), w + (offsetPosition.y * Voxel::Chunk::SIZE), a + (offsetPosition.z * Voxel::Chunk::SIZE), widthSize, 1.0f, heightSize);
        break;

      case FaceType::LEFT:
        Face::Left(vertices, w + (offsetPosition.x * Voxel::Chunk::SIZE), widthOffset + (offsetPosition.y * Voxel::Chunk::SIZE), a + (offsetPosition.z * Voxel::Chunk::SIZE), 1.0f, widthSize, heightSize);
        break;
      case FaceType::RIGHT:
        Face::Right(vertices, w + (offsetPosition.x * Voxel::Chunk::SIZE), widthOffset + (offsetPosition.y * Voxel::Chunk::SIZE), a + (offsetPosition.z * Voxel::Chunk::SIZE), 1.0f, widthSize, heightSize);

        break;
      case FaceType::FRONT:
        Face::Front(vertices, a + (offsetPosition.x * Voxel::Chunk::SIZE), widthOffset + (offsetPosition.y * Voxel::Chunk::SIZE), w + (offsetPosition.z * Voxel::Chunk::SIZE), heightSize, widthSize, 1.0f);
        break;
      case FaceType::BACK:
        Face::Back(vertices, a + (offsetPosition.x * Voxel::Chunk::SIZE), widthOffset + (offsetPosition.y * Voxel::Chunk::SIZE), w + (offsetPosition.z * Voxel::Chunk::SIZE), heightSize, widthSize, 1.0f);
        break;
      }
    }
  }

  static void GreedyMeshAxis(const glm::ivec3 &offsetPosition, const uint32_t (&bits)[], uint32_t (&widthStart)[], uint32_t (&heightStart)[], uint32_t (&widthEnd)[], uint32_t (&heightEnd)[], std::vector<Vertex> &vertices, FaceType startType, FaceType endType)
  {
    for (size_t a = 0; a < Voxel::Chunk::SIZE; a++)
    {
      for (size_t b = 0; b < Voxel::Chunk::SIZE; b++)
      {
        const uint32_t mask = bits[(Voxel::Chunk::SIZE * (b + (Voxel::Chunk::SIZE * a))) / Voxel::Chunk::SIZE];
        GreedyMeshFace(offsetPosition, a, b, mask & ~(mask << 1), widthStart, heightStart, vertices, startType);
        GreedyMeshFace(offsetPosition, a, b, mask & ~(mask >> 1), widthEnd, heightEnd, vertices, endType);
      }
    }
  }

  static void CullMesh(const glm::ivec3 &offsetPosition, std::vector<Vertex> &vertices, const uint32_t (&columns)[], const uint32_t (&rows)[], const uint32_t (&layers)[])
  {
    for (size_t z = 0; z < Voxel::Chunk::SIZE; z++)
    {
      for (size_t x = 0; x < Voxel::Chunk::SIZE; x++)
      {
        uint32_t column = columns[(Voxel::Chunk::SIZE * (x + (Voxel::Chunk::SIZE * z))) / Voxel::Chunk::SIZE];

        while (column)
        {
          const unsigned int offset = __builtin_ffs(column) - 1;
          unsigned int size = __builtin_ctz(~(column >> (__builtin_ffs(column) - 1)));

          column &= ~((1ULL << (size + offset)) - 1);

          Face::Top(vertices, (offsetPosition.x * Voxel::Chunk::SIZE) + x, (offsetPosition.y * Voxel::Chunk::SIZE) + offset, (offsetPosition.z * Voxel::Chunk::SIZE) + z, 1.0f, size, 1.0f);
          Face::Bottom(vertices, (offsetPosition.x * Voxel::Chunk::SIZE) + x, (offsetPosition.y * Voxel::Chunk::SIZE) + offset, (offsetPosition.z * Voxel::Chunk::SIZE) + z, 1.0f, size, 1.0f);
        }
      }
    }

    for (size_t z = 0; z < Voxel::Chunk::SIZE; z++)
    {
      for (size_t y = 0; y < Voxel::Chunk::SIZE; y++)
      {
        uint32_t row = rows[(Voxel::Chunk::SIZE * (y + (Voxel::Chunk::SIZE * z))) / Voxel::Chunk::SIZE];

        while (row)
        {
          const unsigned int offset = __builtin_ffs(row) - 1;
          unsigned int size = __builtin_ctz(~(row >> (__builtin_ffs(row) - 1)));

          row &= ~((1ULL << (size + offset)) - 1);

          Face::Left(vertices, (offsetPosition.x * Voxel::Chunk::SIZE) + offset, (offsetPosition.y * Voxel::Chunk::SIZE) + y, (offsetPosition.z * Voxel::Chunk::SIZE) + z, size, 1.0f, 1.0f);
          Face::Right(vertices, (offsetPosition.x * Voxel::Chunk::SIZE) + offset, (offsetPosition.y * Voxel::Chunk::SIZE) + y, (offsetPosition.z * Voxel::Chunk::SIZE) + z, size, 1.0f, 1.0f);
        }
      }
    }

    for (size_t x = 0; x < Voxel::Chunk::SIZE; x++)
    {
      for (size_t y = 0; y < Voxel::Chunk::SIZE; y++)
      {
        uint32_t depth = layers[(Voxel::Chunk::SIZE * (y + (Voxel::Chunk::SIZE * x))) / Voxel::Chunk::SIZE];

        while (depth)
        {
          const unsigned int offset = __builtin_ffs(depth) - 1;
          unsigned int size = __builtin_ctz(~(depth >> (__builtin_ffs(depth) - 1)));

          depth &= ~((1ULL << (size + offset)) - 1);

          Face::Front(vertices, (offsetPosition.x * Voxel::Chunk::SIZE) + x, (offsetPosition.y * Voxel::Chunk::SIZE) + y, (offsetPosition.z * Voxel::Chunk::SIZE) + offset, 1.0f, 1.0f, size);
          Face::Back(vertices, (offsetPosition.x * Voxel::Chunk::SIZE) + x, (offsetPosition.y * Voxel::Chunk::SIZE) + y, (offsetPosition.z * Voxel::Chunk::SIZE) + offset, 1.0f, 1.0f, size);
        }
      }
    }
  }

public:
  static void Chunk(glm::ivec3 coord, Voxel::Chunk &chunk, std::vector<Vertex> &vertices)
  {
    /**
     * The amount of elements we need to store to capture all voxels;
     * One 32bit integer to capture one column/row/layer. Each bit represents the z/x/z axis.
     * 32 * 32 * 32 = 32768;
     * 32768 / 8 = 4096;
     * 8 bits make one integer hence
     * 4096 integers required to capture 32768 voxels if 1 bit is a voxel.
     */
    unsigned int length = (Voxel::Chunk::SIZE * Voxel::Chunk::SIZE * Voxel::Chunk::SIZE) / 8;

    /**
     * Generate the bit mask for rows, columns and layers.
     *
     * For columns:
     *
     *       At (x, z) you can get the height of the column in one bitwise operation
     *       and you can get the location of all the top & bottom faces in one bitwise operation
     *       y0 y1 y2 y3
     * z0 x0 0  0  0  0
     * z0 x1 0  0  0  0
     * z0 x2 0  0  0  0
     * z0 x3 0  0  0  0
     * z1 x0 0  0  0  0
     * z1 x2 0  0  0  0
     * z1 x3 0  0  0  0
     */
    uint32_t rows[length] = {};
    uint32_t columns[length] = {};
    uint32_t layers[length] = {};

    for (size_t x = 0; x < Voxel::Chunk::SIZE; x++)
      for (size_t y = 0; y < Voxel::Chunk::SIZE; y++)
        for (size_t z = 0; z < Voxel::Chunk::SIZE; z++)
        {
          const Voxel::Voxel &voxel = chunk.get({x, y, z});

          if (voxel.isSolid())
          {
            const unsigned int rowIndex = x + (Voxel::Chunk::SIZE * (y + (Voxel::Chunk::SIZE * z)));
            const unsigned int columnIndex = y + (Voxel::Chunk::SIZE * (x + (Voxel::Chunk::SIZE * z)));
            const unsigned int layerIndex = z + (Voxel::Chunk::SIZE * (y + (Voxel::Chunk::SIZE * x)));

            rows[rowIndex / Voxel::Chunk::SIZE] |= (1ULL << (rowIndex % Voxel::Chunk::SIZE));
            columns[columnIndex / Voxel::Chunk::SIZE] |= (1ULL << (columnIndex % Voxel::Chunk::SIZE));
            layers[layerIndex / Voxel::Chunk::SIZE] |= (1ULL << (layerIndex % Voxel::Chunk::SIZE));
          }
        }

    // CullMesh(coord, vertices, columns, rows, layers);

    /**
     * Culls the column/row/layer
     * From: 001110110011111
     * To: 001010110010001
     *
     * Loop over the correct axis for each column/row/layer
     * for columns x,z
     * for rows    y,z
     * for layers  y,x
     *
     * Set the width and height of each face for both sides (start & end)
     * for columns bottom & top
     * for rows    left & right
     * for layers  front & back
     */

    uint32_t widthStart[length] = {};
    uint32_t heightStart[length] = {};

    uint32_t widthEnd[length] = {};
    uint32_t heightEnd[length] = {};

    // PrepareWidthHeighMasks(columns, widthStart, heightStart, widthEnd, heightEnd);
    // GreedyMeshAxis(coord, columns, widthStart, heightStart, widthEnd, heightEnd, vertices, FaceType::BOTTOM, FaceType::TOP);

    // std::memset(widthStart, 0, sizeof(widthStart));
    // std::memset(heightStart, 0, sizeof(heightStart));
    // std::memset(widthEnd, 0, sizeof(widthEnd));
    // std::memset(heightEnd, 0, sizeof(heightEnd));

    // PrepareWidthHeighMasks(rows, widthStart, heightStart, widthEnd, heightEnd);
    // GreedyMeshAxis(coord, rows, widthStart, heightStart, widthEnd, heightEnd, vertices, FaceType::LEFT, FaceType::RIGHT);

    // std::memset(widthStart, 0, sizeof(widthStart));
    // std::memset(heightStart, 0, sizeof(heightStart));
    // std::memset(widthEnd, 0, sizeof(widthEnd));
    // std::memset(heightEnd, 0, sizeof(heightEnd));

    // PrepareWidthHeighMasks(layers, widthStart, heightStart, widthEnd, heightEnd);
    // GreedyMeshAxis(coord, layers, widthStart, heightStart, widthEnd, heightEnd, vertices, FaceType::FRONT, FaceType::BACK);


    for (size_t z = 0; z < Voxel::Chunk::SIZE; z++)
    {
      for (size_t x = 0; x < Voxel::Chunk::SIZE; x++)
      {
        const uint32_t column = columns[(Voxel::Chunk::SIZE * (x + (Voxel::Chunk::SIZE * z))) / Voxel::Chunk::SIZE];
        uint32_t first = column & ~(column << 1);
        uint32_t last = column & ~(column >> 1);

        while (first)
        {
          const unsigned int w = __builtin_ffs(first) - 1;

          const unsigned int wi = x + (Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * z)));
          widthStart[wi / Voxel::Chunk::SIZE] |= (1ULL << (wi % Voxel::Chunk::SIZE));

          const unsigned int hi = z + (Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * x)));
          heightEnd[hi / Voxel::Chunk::SIZE] |= (1ULL << (hi % Voxel::Chunk::SIZE));

          first &= ~((1ULL << w + 1) - 1);
        }

        while (last)
        {
          const unsigned int w = __builtin_ffs(last) - 1;

          const unsigned int wi = x + (Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * z)));
          widthEnd[wi / Voxel::Chunk::SIZE] |= (1ULL << (wi % Voxel::Chunk::SIZE));

          const unsigned int hi = z + (Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * x)));
          heightEnd[hi / Voxel::Chunk::SIZE] |= (1ULL << (hi % Voxel::Chunk::SIZE));

          last &= ~((1ULL << w + 1) - 1);
        }
      }
    }


    for (size_t x = 0; x < Voxel::Chunk::SIZE; x++)
    {
      for (size_t z = 0; z < Voxel::Chunk::SIZE; z++)
      {
        const uint32_t column = columns[(Voxel::Chunk::SIZE * (x + (Voxel::Chunk::SIZE * z))) / Voxel::Chunk::SIZE];
        uint32_t last = column & ~(column >> 1);

        while (last)
        {
          const unsigned int w = __builtin_ffs(last) - 1;
          last &= ~((1ULL << w + 1) - 1);

          uint32_t &width = widthEnd[(Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * z))) / Voxel::Chunk::SIZE] &= ~((1ULL << x) - 1);

          if (!width)
            continue;

          const unsigned int wOffset = !width ? 0 : __builtin_ffs(width) - 1;
          const unsigned int wSize = __builtin_ctz(~(width >> (__builtin_ffs(width) - 1)));

          uint32_t &height = heightEnd[(Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * (int)(wOffset)))) / Voxel::Chunk::SIZE] &= ~((1ULL << z) - 1);

          const unsigned int hOffset = !height ? 0 : __builtin_ffs(height) - 1;
          unsigned int hSize = __builtin_ctz(~(height >> (__builtin_ffs(height) - 1)));

          for (size_t i = hOffset; i < hOffset + hSize; i++)
          {
            const unsigned int index = (Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * i))) / Voxel::Chunk::SIZE;
            const uint32_t SIZE = widthEnd[index] & (((1ULL << (int)wSize) - 1) << wOffset);

            if (__builtin_ctz(~(SIZE >> (__builtin_ffs(SIZE) - 1))) != wSize)
            {
              hSize = i - hOffset;
              break;
            }

            widthEnd[index] &= ~(((1ULL << (int)wSize) - 1) << wOffset);
          }

          Face::Top(vertices, wOffset + (coord.x * Voxel::Chunk::SIZE), w + (coord.y * Voxel::Chunk::SIZE), z + (coord.z * Voxel::Chunk::SIZE), wSize, 1.0f, hSize);
        }
      }
    }

    std::memset(widthStart, 0, sizeof(widthStart));
    std::memset(heightStart, 0, sizeof(heightStart));
    std::memset(widthEnd, 0, sizeof(widthEnd));
    std::memset(heightEnd, 0, sizeof(heightEnd));



    for (size_t z = 0; z < Voxel::Chunk::SIZE; z++)
    {
      for (size_t y = 0; y < Voxel::Chunk::SIZE; y++)
      {
        const uint32_t row = rows[(Voxel::Chunk::SIZE * (y + (Voxel::Chunk::SIZE * z))) / Voxel::Chunk::SIZE];
        uint32_t first = row & ~(row << 1);
        uint32_t last = row & ~(row >> 1);

        while (first)
        {
          const unsigned int w = __builtin_ffs(first) - 1;

          const unsigned int wi = y + (Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * z)));
          widthStart[wi / Voxel::Chunk::SIZE] |= (1ULL << (wi % Voxel::Chunk::SIZE));

          const unsigned int hi = z + (Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * y)));
          heightStart[hi / Voxel::Chunk::SIZE] |= (1ULL << (hi % Voxel::Chunk::SIZE));

          first &= ~((1ULL << w + 1) - 1);
        }

        while (last)
        {
          const unsigned int w = __builtin_ffs(last) - 1;

          const unsigned int wi = y + (Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * z)));
          widthEnd[wi / Voxel::Chunk::SIZE] |= (1ULL << (wi % Voxel::Chunk::SIZE));

          const unsigned int hi = z + (Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * y)));
          heightEnd[hi / Voxel::Chunk::SIZE] |= (1ULL << (hi % Voxel::Chunk::SIZE));

          last &= ~((1ULL << w + 1) - 1);
        }
      }
    }


     for (size_t z = 0; z < Voxel::Chunk::SIZE; z++)
    {
      for (size_t y = 0; y < Voxel::Chunk::SIZE; y++)
      {
        const uint32_t row = rows[(Voxel::Chunk::SIZE * (y + (Voxel::Chunk::SIZE * z))) / Voxel::Chunk::SIZE];
        uint32_t first = row & ~(row << 1);
        uint32_t last = row & ~(row >> 1);

        while (first)
        {
          const unsigned int w = __builtin_ffs(first) - 1;
          first &= ~((1ULL << w + 1) - 1);

          uint32_t &width = widthStart[(Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * z))) / Voxel::Chunk::SIZE] &= ~((1ULL << y) - 1);

          if (!width)
            continue;

          const unsigned int wOffset = !width ? 0 : __builtin_ffs(width) - 1;
          const unsigned int wSize = __builtin_ctz(~(width >> (__builtin_ffs(width) - 1)));

          uint32_t &height = heightStart[(Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * (int)(wOffset)))) / Voxel::Chunk::SIZE] &= ~((1ULL << z) - 1);

          const unsigned int hOffset = !height ? 0 : __builtin_ffs(height) - 1;
          unsigned int hSize = __builtin_ctz(~(height >> (__builtin_ffs(height) - 1)));

          for (size_t i = hOffset; i < hOffset + hSize; i++)
          {
            const unsigned int index = (Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * i))) / Voxel::Chunk::SIZE;
            const uint32_t SIZE = widthStart[index] & (((1ULL << (int)wSize) - 1) << wOffset);

            if (__builtin_ctz(~(SIZE >> (__builtin_ffs(SIZE) - 1))) != wSize)
            {
              hSize = i - hOffset;
              break;
            }

            widthStart[index] &= ~(((1ULL << (int)wSize) - 1) << wOffset);
          }

          Face::Left(vertices, w + coord.x, wOffset + coord.y, z + coord.z, 1.0f, wSize, hSize);
        }

        while (last)
        {
          const unsigned int w = __builtin_ffs(last) - 1;
          last &= ~((1ULL << w + 1) - 1);

          uint32_t &width = widthEnd[(Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * z))) / Voxel::Chunk::SIZE] &= ~((1ULL << y) - 1);

          if (!width)
            continue;

          const unsigned int wOffset = !width ? 0 : __builtin_ffs(width) - 1;
          const unsigned int wSize = __builtin_ctz(~(width >> (__builtin_ffs(width) - 1)));

          uint32_t &height = heightEnd[(Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * (int)(wOffset)))) / Voxel::Chunk::SIZE] &= ~((1ULL << z) - 1);

          const unsigned int hOffset = !height ? 0 : __builtin_ffs(height) - 1;
          unsigned int hSize = __builtin_ctz(~(height >> (__builtin_ffs(height) - 1)));

          for (size_t i = hOffset; i < hOffset + hSize; i++)
          {
            const unsigned int index = (Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * i))) / Voxel::Chunk::SIZE;
            const uint32_t SIZE = widthEnd[index] & (((1ULL << (int)wSize) - 1) << wOffset);

            if (__builtin_ctz(~(SIZE >> (__builtin_ffs(SIZE) - 1))) != wSize)
            {
              hSize = i - hOffset;
              break;
            }

            widthEnd[index] &= ~(((1ULL << (int)wSize) - 1) << wOffset);
          }

          Face::Right(vertices, w + coord.x, wOffset + coord.y, z + coord.z, 1.0f, wSize, hSize);
        }
      }
    }


    std::memset(widthStart, 0, sizeof(widthStart));
    std::memset(heightStart, 0, sizeof(heightStart));
    std::memset(widthEnd, 0, sizeof(widthEnd));
    std::memset(heightEnd, 0, sizeof(heightEnd));


  for (size_t x = 0; x < Voxel::Chunk::SIZE; x++)
    {
      for (size_t y = 0; y < Voxel::Chunk::SIZE; y++)
      {
        const uint32_t depth = layers[(Voxel::Chunk::SIZE * (y + (Voxel::Chunk::SIZE * x))) / Voxel::Chunk::SIZE];
        uint32_t first = depth & ~(depth << 1);
        uint32_t last = depth & ~(depth >> 1);

        while (first)
        {
          const unsigned int w = __builtin_ffs(first) - 1;

          const unsigned int wi = y + (Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * x)));
          widthStart[wi / Voxel::Chunk::SIZE] |= (1ULL << (wi % Voxel::Chunk::SIZE));

          const unsigned int hi = x + (Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * y)));
          heightStart[hi / Voxel::Chunk::SIZE] |= (1ULL << (hi % Voxel::Chunk::SIZE));

          first &= ~((1ULL << w + 1) - 1);
        }

        while (last)
        {
          const unsigned int w = __builtin_ffs(last) - 1;

          const unsigned int wi = y + (Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * x)));
          widthEnd[wi / Voxel::Chunk::SIZE] |= (1ULL << (wi % Voxel::Chunk::SIZE));

          const unsigned int hi = x + (Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * y)));
          heightEnd[hi / Voxel::Chunk::SIZE] |= (1ULL << (hi % Voxel::Chunk::SIZE));

          last &= ~((1ULL << w + 1) - 1);
        }
      }
    }


    for (size_t x = 0; x < Voxel::Chunk::SIZE; x++)
    {
      for (size_t y = 0; y < Voxel::Chunk::SIZE; y++)
      {
        const uint32_t depth = layers[(Voxel::Chunk::SIZE * (y + (Voxel::Chunk::SIZE * x))) / Voxel::Chunk::SIZE];
        uint32_t first = depth & ~(depth << 1);
        uint32_t last = depth & ~(depth >> 1);

        while (first)
        {
          const unsigned int w = __builtin_ffs(first) - 1;
          first &= ~((1ULL << w + 1) - 1);

          uint32_t &width = widthStart[(Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * x))) / Voxel::Chunk::SIZE] &= ~((1ULL << y) - 1);

          if (!width)
            continue;

          const unsigned int wOffset = !width ? 0 : __builtin_ffs(width) - 1;
          const unsigned int wSize = __builtin_ctz(~(width >> (__builtin_ffs(width) - 1)));

          uint32_t &height = heightEnd[(Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * (int)(wOffset)))) / Voxel::Chunk::SIZE] &= ~((1ULL << x) - 1);

          const unsigned int hOffset = !height ? 0 : __builtin_ffs(height) - 1;
          unsigned int hSize = __builtin_ctz(~(height >> (__builtin_ffs(height) - 1)));

          for (size_t i = hOffset; i < hOffset + hSize; i++)
          {
            const unsigned int index = (Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * i))) / Voxel::Chunk::SIZE;
            const uint32_t SIZE = widthStart[index] & (((1ULL << (int)wSize) - 1) << wOffset);

            if (__builtin_ctz(~(SIZE >> (__builtin_ffs(SIZE) - 1))) != wSize)
            {
              hSize = i - hOffset;
              break;
            }

            widthStart[index] &= ~(((1ULL << (int)wSize) - 1) << wOffset);
          }

          Face::Front(vertices, x + coord.x, wOffset + coord.y, w + coord.z, hSize, wSize, 1.0f);
        }

        while (last)
        {
          const unsigned int w = __builtin_ffs(last) - 1;
          last &= ~((1ULL << w + 1) - 1);

          uint32_t &width = widthEnd[(Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * x))) / Voxel::Chunk::SIZE] &= ~((1ULL << y) - 1);

          if (!width)
            continue;

          const unsigned int wOffset = !width ? 0 : __builtin_ffs(width) - 1;
          const unsigned int wSize = __builtin_ctz(~(width >> (__builtin_ffs(width) - 1)));

          uint32_t &height = heightEnd[(Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * (int)(wOffset)))) / Voxel::Chunk::SIZE] &= ~((1ULL << x) - 1);

          const unsigned int hOffset = !height ? 0 : __builtin_ffs(height) - 1;
          unsigned int hSize = __builtin_ctz(~(height >> (__builtin_ffs(height) - 1)));

          for (size_t i = hOffset; i < hOffset + hSize; i++)
          {
            const unsigned int index = (Voxel::Chunk::SIZE * (w + (Voxel::Chunk::SIZE * i))) / Voxel::Chunk::SIZE;
            const uint32_t SIZE = widthEnd[index] & (((1ULL << (int)wSize) - 1) << wOffset);

            if (__builtin_ctz(~(SIZE >> (__builtin_ffs(SIZE) - 1))) != wSize)
            {
              hSize = i - hOffset;
              break;
            }

            widthEnd[index] &= ~(((1ULL << (int)wSize) - 1) << wOffset);
          }

          Face::Back(vertices, x + coord.x, wOffset + coord.y, w + coord.z, hSize, wSize, 1.0f);
        }
      }
    }
  }
};