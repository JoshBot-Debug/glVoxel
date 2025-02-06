#pragma once

#include <vector>
#include <bitset>

#include <glm/glm.hpp>

#include "Voxel.h"
#include "Engine/Types.h"
#include "Engine/Face.h"

constexpr unsigned int ChunkSize = Voxel::Chunk::SIZE;

/**
 * The amount of elements we need to store to capture all voxels;
 * One 32bit integer to capture one column/row/layer. Each bit represents the z/x/z axis.
 * 32 * 32 * 32 = 32768;
 * 32768 / 8 = 4096;
 * 8 bits make one integer hence
 * 4096 integers required to capture 32768 voxels if 1 bit is a voxel.
 */
constexpr unsigned int MaskLength = (ChunkSize * ChunkSize * ChunkSize) / 8;

class GreedyMesh
{
private:
  static void SetWidthHeight(unsigned int a, unsigned int b, uint32_t bits, uint32_t (&widthMasks)[], uint32_t (&heightMasks)[])
  {
    while (bits)
    {
      const unsigned int w = __builtin_ffs(bits) - 1;

      const unsigned int wi = a + (ChunkSize * (w + (ChunkSize * b)));
      widthMasks[wi / ChunkSize] |= (1ULL << (wi % ChunkSize));

      const unsigned int hi = b + (ChunkSize * (w + (ChunkSize * a)));
      heightMasks[hi / ChunkSize] |= (1ULL << (hi % ChunkSize));

      bits &= ~((1ULL << w + 1) - 1);
    }
  }

  static void PrepareWidthHeighMasks(const uint32_t (&bits)[], uint32_t (&widthStart)[], uint32_t (&heightStart)[], uint32_t (&widthEnd)[], uint32_t (&heightEnd)[])
  {
    for (size_t a = 0; a < ChunkSize; a++)
    {
      for (size_t b = 0; b < ChunkSize; b++)
      {
        const uint32_t mask = bits[(ChunkSize * (a + (ChunkSize * b))) / ChunkSize];
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

      const uint32_t &width = widthMasks[(ChunkSize * (w + (ChunkSize * a))) / ChunkSize] &= ~((1ULL << b) - 1);

      if (!width)
        continue;

      const unsigned int widthOffset = !width ? 0 : __builtin_ffs(width) - 1;
      const unsigned int widthSize = __builtin_ctz(~(width >> (__builtin_ffs(width) - 1)));

      const uint32_t &height = heightMasks[(ChunkSize * (w + (ChunkSize * (int)(widthOffset)))) / ChunkSize] &= ~((1ULL << a) - 1);

      const unsigned int heightOffset = __builtin_ffs(height) - 1;
      unsigned int heightSize = __builtin_ctz(~(height >> heightOffset));

      for (size_t i = heightOffset; i < heightOffset + heightSize; i++)
      {
        const unsigned int index = (ChunkSize * (w + (ChunkSize * i))) / ChunkSize;
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
        Face::Top(vertices, widthOffset + (offsetPosition.x * ChunkSize), w + (offsetPosition.y * ChunkSize), a + (offsetPosition.z * ChunkSize), widthSize, 1.0f, heightSize);
        break;
      case FaceType::BOTTOM:
        Face::Bottom(vertices, widthOffset + (offsetPosition.x * ChunkSize), w + (offsetPosition.y * ChunkSize), a + (offsetPosition.z * ChunkSize), widthSize, 1.0f, heightSize);
        break;

      case FaceType::LEFT:
        Face::Left(vertices, w + (offsetPosition.x * ChunkSize), widthOffset + (offsetPosition.y * ChunkSize), a + (offsetPosition.z * ChunkSize), 1.0f, widthSize, heightSize);
        break;
      case FaceType::RIGHT:
        Face::Right(vertices, w + (offsetPosition.x * ChunkSize), widthOffset + (offsetPosition.y * ChunkSize), a + (offsetPosition.z * ChunkSize), 1.0f, widthSize, heightSize);

        break;
      case FaceType::FRONT:
        Face::Front(vertices, a + (offsetPosition.x * ChunkSize), widthOffset + (offsetPosition.y * ChunkSize), w + (offsetPosition.z * ChunkSize), heightSize, widthSize, 1.0f);
        break;
      case FaceType::BACK:
        Face::Back(vertices, a + (offsetPosition.x * ChunkSize), widthOffset + (offsetPosition.y * ChunkSize), w + (offsetPosition.z * ChunkSize), heightSize, widthSize, 1.0f);
        break;
      }
    }
  }

  static void GreedyMeshAxis(const glm::ivec3 &offsetPosition, const uint32_t (&bits)[], uint32_t (&widthStart)[], uint32_t (&heightStart)[], uint32_t (&widthEnd)[], uint32_t (&heightEnd)[], std::vector<Vertex> &vertices, FaceType startType, FaceType endType)
  {
    for (size_t a = 0; a < ChunkSize; a++)
    {
      for (size_t b = 0; b < ChunkSize; b++)
      {
        const uint32_t mask = bits[(ChunkSize * (b + (ChunkSize * a))) / ChunkSize];
        GreedyMeshFace(offsetPosition, a, b, mask & ~(mask << 1), widthStart, heightStart, vertices, startType);
        GreedyMeshFace(offsetPosition, a, b, mask & ~(mask >> 1), widthEnd, heightEnd, vertices, endType);
      }
    }
  }

  static void CullMesh(const glm::ivec3 &offsetPosition, std::vector<Vertex> &vertices, uint32_t (&columns)[], uint32_t (&rows)[], uint32_t (&layers)[])
  {
    for (size_t a = 0; a < ChunkSize; a++)
    {
      for (size_t b = 0; b < ChunkSize; b++)
      {
        uint32_t &column = columns[(ChunkSize * (b + (ChunkSize * a))) / ChunkSize];
        uint32_t &row = rows[(ChunkSize * (b + (ChunkSize * a))) / ChunkSize];
        uint32_t &depth = layers[(ChunkSize * (b + (ChunkSize * a))) / ChunkSize];

        while (column)
        {
          const unsigned int offset = __builtin_ffs(column) - 1;
          unsigned int size = __builtin_ctz(~(column >> (__builtin_ffs(column) - 1)));

          column &= ~((1ULL << (size + offset)) - 1);

          Face::Top(vertices, (offsetPosition.x * ChunkSize) + b, (offsetPosition.y * ChunkSize) + offset, (offsetPosition.z * ChunkSize) + a, 1.0f, size, 1.0f);
          Face::Bottom(vertices, (offsetPosition.x * ChunkSize) + b, (offsetPosition.y * ChunkSize) + offset, (offsetPosition.z * ChunkSize) + a, 1.0f, size, 1.0f);
        }

        while (row)
        {
          const unsigned int offset = __builtin_ffs(row) - 1;
          unsigned int size = __builtin_ctz(~(row >> (__builtin_ffs(row) - 1)));

          row &= ~((1ULL << (size + offset)) - 1);

          Face::Left(vertices, (offsetPosition.x * ChunkSize) + offset, (offsetPosition.y * ChunkSize) + b, (offsetPosition.z * ChunkSize) + a, size, 1.0f, 1.0f);
          Face::Right(vertices, (offsetPosition.x * ChunkSize) + offset, (offsetPosition.y * ChunkSize) + b, (offsetPosition.z * ChunkSize) + a, size, 1.0f, 1.0f);
        }

        while (depth)
        {
          const unsigned int offset = __builtin_ffs(depth) - 1;
          unsigned int size = __builtin_ctz(~(depth >> (__builtin_ffs(depth) - 1)));

          depth &= ~((1ULL << (size + offset)) - 1);

          Face::Front(vertices, (offsetPosition.x * ChunkSize) + a, (offsetPosition.y * ChunkSize) + b, (offsetPosition.z * ChunkSize) + offset, 1.0f, 1.0f, size);
          Face::Back(vertices, (offsetPosition.x * ChunkSize) + a, (offsetPosition.y * ChunkSize) + b, (offsetPosition.z * ChunkSize) + offset, 1.0f, 1.0f, size);
        }
      }
    }
  }

public:
  static void Chunk(glm::ivec3 coord, Voxel::Chunk &chunk, std::vector<Vertex> &vertices)
  {
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
    uint32_t rows[MaskLength] = {};
    uint32_t columns[MaskLength] = {};
    uint32_t layers[MaskLength] = {};

    for (size_t x = 0; x < ChunkSize; x++)
      for (size_t y = 0; y < ChunkSize; y++)
        for (size_t z = 0; z < ChunkSize; z++)
        {
          const Voxel::Voxel &voxel = chunk.get({x, y, z});

          if (voxel.isSolid())
          {
            const unsigned int rowIndex = x + (ChunkSize * (y + (ChunkSize * z)));
            const unsigned int columnIndex = y + (ChunkSize * (x + (ChunkSize * z)));
            const unsigned int layerIndex = z + (ChunkSize * (y + (ChunkSize * x)));

            rows[rowIndex / ChunkSize] |= (1ULL << (rowIndex % ChunkSize));
            columns[columnIndex / ChunkSize] |= (1ULL << (columnIndex % ChunkSize));
            layers[layerIndex / ChunkSize] |= (1ULL << (layerIndex % ChunkSize));
          }
        }

    /**
     * Cull meshing, ~0.13ms slower than greedy meshing
     * 
     * CullMesh(coord, vertices, columns, rows, layers);
     */

    uint32_t widthStart[MaskLength] = {};
    uint32_t heightStart[MaskLength] = {};

    uint32_t widthEnd[MaskLength] = {};
    uint32_t heightEnd[MaskLength] = {};

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
    PrepareWidthHeighMasks(columns, widthStart, heightStart, widthEnd, heightEnd);

    /**
     * Greedy mesh the given axis
     */
    GreedyMeshAxis(coord, columns, widthStart, heightStart, widthEnd, heightEnd, vertices, FaceType::BOTTOM, FaceType::TOP);

    std::memset(widthStart, 0, sizeof(widthStart));
    std::memset(heightStart, 0, sizeof(heightStart));
    std::memset(widthEnd, 0, sizeof(widthEnd));
    std::memset(heightEnd, 0, sizeof(heightEnd));

    PrepareWidthHeighMasks(rows, widthStart, heightStart, widthEnd, heightEnd);
    GreedyMeshAxis(coord, rows, widthStart, heightStart, widthEnd, heightEnd, vertices, FaceType::LEFT, FaceType::RIGHT);

    std::memset(widthStart, 0, sizeof(widthStart));
    std::memset(heightStart, 0, sizeof(heightStart));
    std::memset(widthEnd, 0, sizeof(widthEnd));
    std::memset(heightEnd, 0, sizeof(heightEnd));

    PrepareWidthHeighMasks(layers, widthStart, heightStart, widthEnd, heightEnd);
    GreedyMeshAxis(coord, layers, widthStart, heightStart, widthEnd, heightEnd, vertices, FaceType::FRONT, FaceType::BACK);
  }
};