#pragma once

#include <vector>
#include <bitset>
#include <cstring>

#include <glm/glm.hpp>

#include "Voxel.h"
#include "Engine/Types.h"
#include "Engine/Face.h"

#include <immintrin.h>

constexpr unsigned int ChunkSize = Voxel::Chunk::ChunkSize;

/**
 * The amount of elements we need to store to capture all voxels;
 * One 32bit integer to capture one column/row/layer. Each bit represents the z/x/z axis.
 * 32 * 32 * 32 = 32768;
 * 32768 / 8 = 4096;
 * 8 bits make one integer hence
 * 4096 integers required to capture 32768 voxels if 1 bit is a voxel.
 */
constexpr unsigned int MaskLength = Voxel::Chunk::MaskLength;

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

  static void PrepareWidthHeightMasks(const uint64_t (&bits)[], uint32_t (&widthStart)[], uint32_t (&heightStart)[], uint32_t (&widthEnd)[], uint32_t (&heightEnd)[])
  {
    for (uint8_t a = 0; a < ChunkSize; a++)
      for (uint8_t b = 0; b < ChunkSize; b++)
      {
        /**
         * Get the bitmask at index a,b
         * The padding mask has an extra bit as the LSB and MSB.
         * The MSB is the LSB of the pervious neighbour chunk
         * The LSB is the MSB of the next neighbour chunk
         * The first & last will always be a zero because there is no neighbour next to them.
         * 0...1 => 1...1 => 1...0
         */
        const uint64_t paddingMask = bits[(ChunkSize * (a + (ChunkSize * b))) / ChunkSize];

        /**
         * Shift right to remove the MSB padding bit and extract the following 32bits into a new mask
         * This is the actual mask we will use for the height and width
         */
        const uint32_t mask = (paddingMask >> 1) & 0xFFFFFFFF;

        /**
         * Remove all the bits other than the start face
         * 11100111100011 => 00100000100001
         */
        uint32_t startMask = mask & ~(mask << 1);

        /**
         * Likewise remove all the bits other than the end face
         * 11100111100011 => 10000100000010
         */
        uint32_t endMask = mask & ~(mask >> 1);

        /**
         * Check the padding mask, if the bit at 0 index is on
         * turn off the MSB of the start mask
         */
        if ((paddingMask >> 0) & 1)
          startMask &= ~(1ULL << 0);

        /**
         * Check the padding mask, if the bit at 33 index is on
         * turn off the LSB of the end mask
         * 
         * This is done in order to not set the height & width of the face at the end of the chunk if the neighbour is the same
         * To avoid creating faces inbetween chunks
         */
        if ((paddingMask >> 33) & 1)
          endMask &= ~(1ULL << 31);

        SetWidthHeight(a, b, startMask, widthStart, heightStart);
        SetWidthHeight(a, b, endMask, widthEnd, heightEnd);
      }
  }

  static void GreedyMeshFace(const glm::ivec3 &offsetPosition, uint8_t a, uint8_t b, uint64_t bits, uint32_t (&widthMasks)[], uint32_t (&heightMasks)[], std::vector<Vertex> &vertices, FaceType type)
  {
    while (bits)
    {
      const unsigned int w = __builtin_ffs(bits) - 1;
      bits &= ~((1ULL << w + 1) - 1);

      const uint32_t &width = widthMasks[(ChunkSize * (w + (ChunkSize * a))) / ChunkSize] &= ~((1ULL << b) - 1);

      if (!width)
        continue;

      const unsigned int widthOffset = __builtin_ffs(width) - 1;
      const unsigned int widthSize = __builtin_ctz(~(width >> widthOffset));

      const uint32_t &height = heightMasks[(ChunkSize * (w + (ChunkSize * (int)(widthOffset)))) / ChunkSize] &= ~((1ULL << a) - 1);

      const unsigned int heightOffset = __builtin_ffs(height) - 1;
      unsigned int heightSize = __builtin_ctz(~(height >> heightOffset));

      for (uint8_t i = heightOffset; i < heightOffset + heightSize; i++)
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

  static void GreedyMeshAxis(const glm::ivec3 &offsetPosition, const uint64_t (&bits)[], uint32_t (&widthStart)[], uint32_t (&heightStart)[], uint32_t (&widthEnd)[], uint32_t (&heightEnd)[], std::vector<Vertex> &vertices, FaceType startType, FaceType endType)
  {
    for (uint8_t a = 0; a < ChunkSize; a++)
      for (uint8_t b = 0; b < ChunkSize; b++)
      {
        const uint32_t mask = (bits[(ChunkSize * (b + (ChunkSize * a))) / ChunkSize] >> 1) & 0xFFFFFFFF;
        GreedyMeshFace(offsetPosition, a, b, mask & ~(mask << 1), widthStart, heightStart, vertices, startType);
        GreedyMeshFace(offsetPosition, a, b, mask & ~(mask >> 1), widthEnd, heightEnd, vertices, endType);
      }
  }

  static void CullMesh(const glm::ivec3 &offsetPosition, std::vector<Vertex> &vertices, uint32_t (&columns)[], uint32_t (&rows)[], uint32_t (&layers)[])
  {
    for (uint8_t a = 0; a < ChunkSize; a++)
    {
      for (uint8_t b = 0; b < ChunkSize; b++)
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
  static void Chunk(glm::ivec3 coord, Voxel::Chunk &chunk, std::vector<Vertex> &vertices, std::vector<Voxel::Chunk *> &neighbours)
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
     * z1 x1 0  0  0  0
     * z1 x2 0  0  0  0
     * z1 x3 0  0  0  0
     */
    uint64_t rows[MaskLength] = {};
    uint64_t columns[MaskLength] = {};
    uint64_t layers[MaskLength] = {};

    for (uint8_t x = 0; x < ChunkSize; x++)
      for (uint8_t y = 0; y < ChunkSize; y++)
        for (uint8_t z = 0; z < ChunkSize; z++)
        {
          Voxel::Voxel &voxel = chunk.get({x, y, z});

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
     * Here we capture the padding bit
     * Every chunk we need to get the neighbour chunks and check
     * if the bits at the 0 & 31st index are on. If so, we need to skip making
     * faces on that end.
     */
    Voxel::Chunk *right = neighbours[0];
    Voxel::Chunk *left = neighbours[1];

    Voxel::Chunk *top = neighbours[2];
    Voxel::Chunk *bottom = neighbours[3];

    Voxel::Chunk *front = neighbours[4];
    Voxel::Chunk *back = neighbours[5];

    for (uint16_t i = 0; i < MaskLength; i++)
    {
      uint64_t &row = rows[i];
      uint64_t &column = columns[i];
      uint64_t &layer = layers[i];

      row = (row << 1);
      column = (column << 1);
      layer = (layer << 1);

      int fast = i % ChunkSize;
      int slow = (i / ChunkSize) % ChunkSize;

      if (right && right->get({0, fast, slow}).isSolid())
        row |= (1ULL << 33);

      if (left && left->get({31, fast, slow}).isSolid())
        row |= (1ULL << 0);

      if (top && top->get({fast, 0, slow}).isSolid())
        column |= (1ULL << 33);

      if (bottom && bottom->get({fast, 31, slow}).isSolid())
        column |= (1ULL << 0);

      if (front && front->get({slow, fast, 0}).isSolid())
        layer |= (1ULL << 33);

      if (back && back->get({slow, fast, 31}).isSolid())
        layer |= (1ULL << 0);
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
    PrepareWidthHeightMasks(columns, widthStart, heightStart, widthEnd, heightEnd);

    /**
     * Greedy mesh the given axis
     */
    GreedyMeshAxis(coord, columns, widthStart, heightStart, widthEnd, heightEnd, vertices, FaceType::BOTTOM, FaceType::TOP);

    std::memset(widthStart, 0, sizeof(widthStart));
    std::memset(heightStart, 0, sizeof(heightStart));
    std::memset(widthEnd, 0, sizeof(widthEnd));
    std::memset(heightEnd, 0, sizeof(heightEnd));

    PrepareWidthHeightMasks(rows, widthStart, heightStart, widthEnd, heightEnd);
    GreedyMeshAxis(coord, rows, widthStart, heightStart, widthEnd, heightEnd, vertices, FaceType::LEFT, FaceType::RIGHT);

    std::memset(widthStart, 0, sizeof(widthStart));
    std::memset(heightStart, 0, sizeof(heightStart));
    std::memset(widthEnd, 0, sizeof(widthEnd));
    std::memset(heightEnd, 0, sizeof(heightEnd));

    PrepareWidthHeightMasks(layers, widthStart, heightStart, widthEnd, heightEnd);
    GreedyMeshAxis(coord, layers, widthStart, heightStart, widthEnd, heightEnd, vertices, FaceType::FRONT, FaceType::BACK);
  }
};