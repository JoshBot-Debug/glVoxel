#pragma once

#include <vector>
#include <bitset>
#include <cstring>

#include <glm/glm.hpp>

#include "SparseVoxelOctree.h"
#include "Engine/Types.h"
#include "Engine/Face.h"

#include <immintrin.h>

class GreedyMesh
{
private:
  static void SetWidthHeight(unsigned int a, unsigned int b, uint32_t bits, uint32_t (&widthMasks)[], uint32_t (&heightMasks)[], unsigned int chunkSize)
  {
    while (bits)
    {
      const unsigned int w = __builtin_ffs(bits) - 1;

      const unsigned int wi = a + (chunkSize * (w + (chunkSize * b)));
      widthMasks[wi / chunkSize] |= (1ULL << (wi % chunkSize));

      const unsigned int hi = b + (chunkSize * (w + (chunkSize * a)));
      heightMasks[hi / chunkSize] |= (1ULL << (hi % chunkSize));

      bits &= ~((1ULL << w + 1) - 1);
    }
  }

  static void PrepareWidthHeightMasks(const uint64_t (&bits)[], uint32_t (&widthStart)[], uint32_t (&heightStart)[], uint32_t (&widthEnd)[], uint32_t (&heightEnd)[], unsigned int chunkSize)
  {
    for (uint8_t a = 0; a < chunkSize; a++)
      for (uint8_t b = 0; b < chunkSize; b++)
      {
        /**
         * Get the bitmask at index a,b
         * The padding mask has an extra bit as the LSB and MSB.
         * The MSB is the LSB of the pervious neighbour chunk
         * The LSB is the MSB of the next neighbour chunk
         * The first & last will always be a zero because there is no neighbour next to them.
         * 0...1 => 1...1 => 1...0
         */
        const uint64_t paddingMask = bits[(chunkSize * (a + (chunkSize * b))) / chunkSize];

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

        SetWidthHeight(a, b, startMask, widthStart, heightStart, chunkSize);
        SetWidthHeight(a, b, endMask, widthEnd, heightEnd, chunkSize);
      }
  }

  static void GreedyMeshFace(const glm::ivec3 &offsetPosition, uint8_t a, uint8_t b, uint64_t bits, uint32_t (&widthMasks)[], uint32_t (&heightMasks)[], std::vector<Vertex> &vertices, FaceType type, unsigned int chunkSize)
  {
    while (bits)
    {
      const unsigned int w = __builtin_ffs(bits) - 1;
      bits &= ~((1ULL << w + 1) - 1);

      const uint32_t &width = widthMasks[(chunkSize * (w + (chunkSize * a))) / chunkSize] &= ~((1ULL << b) - 1);

      if (!width)
        continue;

      const unsigned int widthOffset = __builtin_ffs(width) - 1;
      const unsigned int widthSize = __builtin_ctz(~(width >> widthOffset));

      const uint32_t &height = heightMasks[(chunkSize * (w + (chunkSize * (int)(widthOffset)))) / chunkSize] &= ~((1ULL << a) - 1);

      const unsigned int heightOffset = __builtin_ffs(height) - 1;
      unsigned int heightSize = __builtin_ctz(~(height >> heightOffset));

      for (uint8_t i = heightOffset; i < heightOffset + heightSize; i++)
      {
        const unsigned int index = (chunkSize * (w + (chunkSize * i))) / chunkSize;
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
        Face::Top(vertices, widthOffset + (offsetPosition.x * chunkSize), w + (offsetPosition.y * chunkSize), a + (offsetPosition.z * chunkSize), widthSize, 1.0f, heightSize);
        break;
      case FaceType::BOTTOM:
        Face::Bottom(vertices, widthOffset + (offsetPosition.x * chunkSize), w + (offsetPosition.y * chunkSize), a + (offsetPosition.z * chunkSize), widthSize, 1.0f, heightSize);
        break;

      case FaceType::LEFT:
        Face::Left(vertices, w + (offsetPosition.x * chunkSize), widthOffset + (offsetPosition.y * chunkSize), a + (offsetPosition.z * chunkSize), 1.0f, widthSize, heightSize);
        break;
      case FaceType::RIGHT:
        Face::Right(vertices, w + (offsetPosition.x * chunkSize), widthOffset + (offsetPosition.y * chunkSize), a + (offsetPosition.z * chunkSize), 1.0f, widthSize, heightSize);

        break;
      case FaceType::FRONT:
        Face::Front(vertices, a + (offsetPosition.x * chunkSize), widthOffset + (offsetPosition.y * chunkSize), w + (offsetPosition.z * chunkSize), heightSize, widthSize, 1.0f);
        break;
      case FaceType::BACK:
        Face::Back(vertices, a + (offsetPosition.x * chunkSize), widthOffset + (offsetPosition.y * chunkSize), w + (offsetPosition.z * chunkSize), heightSize, widthSize, 1.0f);
        break;
      }
    }
  }

  static void GreedyMeshAxis(const glm::ivec3 &offsetPosition, const uint64_t (&bits)[], uint32_t (&widthStart)[], uint32_t (&heightStart)[], uint32_t (&widthEnd)[], uint32_t (&heightEnd)[], std::vector<Vertex> &vertices, FaceType startType, FaceType endType, unsigned int chunkSize)
  {
    for (uint8_t a = 0; a < chunkSize; a++)
      for (uint8_t b = 0; b < chunkSize; b++)
      {
        const uint32_t mask = (bits[(chunkSize * (b + (chunkSize * a))) / chunkSize] >> 1) & 0xFFFFFFFF;
        GreedyMeshFace(offsetPosition, a, b, mask & ~(mask << 1), widthStart, heightStart, vertices, startType, chunkSize);
        GreedyMeshFace(offsetPosition, a, b, mask & ~(mask >> 1), widthEnd, heightEnd, vertices, endType, chunkSize);
      }
  }

  static void CullMesh(const glm::ivec3 &offsetPosition, std::vector<Vertex> &vertices, uint64_t (&columns)[], uint64_t (&rows)[], uint64_t (&layers)[], unsigned int chunkSize)
  {
    for (uint8_t a = 0; a < chunkSize; a++)
    {
      for (uint8_t b = 0; b < chunkSize; b++)
      {
        uint64_t &column = columns[(chunkSize * (b + (chunkSize * a))) / chunkSize];
        uint64_t &row = rows[(chunkSize * (b + (chunkSize * a))) / chunkSize];
        uint64_t &depth = layers[(chunkSize * (b + (chunkSize * a))) / chunkSize];

        while (column)
        {
          const unsigned int offset = __builtin_ffs(column) - 1;
          unsigned int size = __builtin_ctz(~(column >> (__builtin_ffs(column) - 1)));

          column &= ~((1ULL << (size + offset)) - 1);

          Face::Top(vertices, (offsetPosition.x * chunkSize) + b, (offsetPosition.y * chunkSize) + offset, (offsetPosition.z * chunkSize) + a, 1.0f, size, 1.0f);
          Face::Bottom(vertices, (offsetPosition.x * chunkSize) + b, (offsetPosition.y * chunkSize) + offset, (offsetPosition.z * chunkSize) + a, 1.0f, size, 1.0f);
        }

        while (row)
        {
          const unsigned int offset = __builtin_ffs(row) - 1;
          unsigned int size = __builtin_ctz(~(row >> (__builtin_ffs(row) - 1)));

          row &= ~((1ULL << (size + offset)) - 1);

          Face::Left(vertices, (offsetPosition.x * chunkSize) + offset, (offsetPosition.y * chunkSize) + b, (offsetPosition.z * chunkSize) + a, size, 1.0f, 1.0f);
          Face::Right(vertices, (offsetPosition.x * chunkSize) + offset, (offsetPosition.y * chunkSize) + b, (offsetPosition.z * chunkSize) + a, size, 1.0f, 1.0f);
        }

        while (depth)
        {
          const unsigned int offset = __builtin_ffs(depth) - 1;
          unsigned int size = __builtin_ctz(~(depth >> (__builtin_ffs(depth) - 1)));

          depth &= ~((1ULL << (size + offset)) - 1);

          Face::Front(vertices, (offsetPosition.x * chunkSize) + a, (offsetPosition.y * chunkSize) + b, (offsetPosition.z * chunkSize) + offset, 1.0f, 1.0f, size);
          Face::Back(vertices, (offsetPosition.x * chunkSize) + a, (offsetPosition.y * chunkSize) + b, (offsetPosition.z * chunkSize) + offset, 1.0f, 1.0f, size);
        }
      }
    }
  }

public:
  static void Chunk(const glm::ivec3 &coord, Voxel::Chunk &chunk, std::vector<Vertex> &vertices, std::vector<Voxel::Chunk *> &neighbours, unsigned int chunkSize, unsigned int maskLength)
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
    uint64_t rows[maskLength] = {};
    uint64_t columns[maskLength] = {};
    uint64_t layers[maskLength] = {};

    for (uint8_t x = 0; x < chunkSize; x++)
      for (uint8_t y = 0; y < chunkSize; y++)
        for (uint8_t z = 0; z < chunkSize; z++)
        {
          Voxel::Voxel &voxel = chunk.get({x, y, z});

          if (voxel.isSolid())
          {
            const unsigned int rowIndex = x + (chunkSize * (y + (chunkSize * z)));
            const unsigned int columnIndex = y + (chunkSize * (x + (chunkSize * z)));
            const unsigned int layerIndex = z + (chunkSize * (y + (chunkSize * x)));

            rows[rowIndex / chunkSize] |= (1ULL << (rowIndex % chunkSize));
            columns[columnIndex / chunkSize] |= (1ULL << (columnIndex % chunkSize));
            layers[layerIndex / chunkSize] |= (1ULL << (layerIndex % chunkSize));
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

    for (uint16_t i = 0; i < maskLength; i++)
    {
      uint64_t &row = rows[i];
      uint64_t &column = columns[i];
      uint64_t &layer = layers[i];

      row = (row << 1);
      column = (column << 1);
      layer = (layer << 1);

      int fast = i % chunkSize;
      int slow = (i / chunkSize) % chunkSize;

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

    uint32_t widthStart[maskLength] = {};
    uint32_t heightStart[maskLength] = {};

    uint32_t widthEnd[maskLength] = {};
    uint32_t heightEnd[maskLength] = {};

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
    PrepareWidthHeightMasks(columns, widthStart, heightStart, widthEnd, heightEnd, chunkSize);

    /**
     * Greedy mesh the given axis
     */
    GreedyMeshAxis(coord, columns, widthStart, heightStart, widthEnd, heightEnd, vertices, FaceType::BOTTOM, FaceType::TOP, chunkSize);

    std::memset(widthStart, 0, sizeof(widthStart));
    std::memset(heightStart, 0, sizeof(heightStart));
    std::memset(widthEnd, 0, sizeof(widthEnd));
    std::memset(heightEnd, 0, sizeof(heightEnd));

    PrepareWidthHeightMasks(rows, widthStart, heightStart, widthEnd, heightEnd, chunkSize);
    GreedyMeshAxis(coord, rows, widthStart, heightStart, widthEnd, heightEnd, vertices, FaceType::LEFT, FaceType::RIGHT, chunkSize);

    std::memset(widthStart, 0, sizeof(widthStart));
    std::memset(heightStart, 0, sizeof(heightStart));
    std::memset(widthEnd, 0, sizeof(widthEnd));
    std::memset(heightEnd, 0, sizeof(heightEnd));

    PrepareWidthHeightMasks(layers, widthStart, heightStart, widthEnd, heightEnd, chunkSize);
    GreedyMeshAxis(coord, layers, widthStart, heightStart, widthEnd, heightEnd, vertices, FaceType::FRONT, FaceType::BACK, chunkSize);
  }

  static void SparseVoxelTree(Voxel::SparseVoxelOctree *tree, std::vector<Vertex> &vertices, int originX, int originY, int originZ, unsigned int chunkSize, unsigned int maskLength)
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
    uint64_t rows[maskLength] = {};
    uint64_t columns[maskLength] = {};
    uint64_t layers[maskLength] = {};

    for (uint8_t x = 0; x < chunkSize; x++)
      for (uint8_t y = 0; y < chunkSize; y++)
        for (uint8_t z = 0; z < chunkSize; z++)
          if (tree->get(x + originX, y + originY, z + originZ))
          {
            const unsigned int rowIndex = x + (chunkSize * (y + (chunkSize * z)));
            const unsigned int columnIndex = y + (chunkSize * (x + (chunkSize * z)));
            const unsigned int layerIndex = z + (chunkSize * (y + (chunkSize * x)));

            rows[rowIndex / chunkSize] |= (1ULL << (rowIndex % chunkSize));
            columns[columnIndex / chunkSize] |= (1ULL << (columnIndex % chunkSize));
            layers[layerIndex / chunkSize] |= (1ULL << (layerIndex % chunkSize));
          }

    /**
     * Here we capture the padding bit
     * Every chunk we need to get the neighbour chunks and check
     * if the bits at the 0 & 31st index are on. If so, we need to skip making
     * faces on that end.
     */
    for (uint16_t i = 0; i < maskLength; i++)
    {
      uint64_t &row = rows[i];
      uint64_t &column = columns[i];
      uint64_t &layer = layers[i];

      row = (row << 1);
      column = (column << 1);
      layer = (layer << 1);

      int fast = i % chunkSize;
      int slow = (i / chunkSize) % chunkSize;

      if (tree->get(originX + 32, fast + originY, slow + originZ))
        row |= (1ULL << 33);

      if (tree->get(originX - 1, fast + originY, slow + originZ))
        row |= (1ULL << 0);

      if (tree->get(fast + originX, originY + 32, slow + originZ))
        column |= (1ULL << 33);

      if (tree->get(fast + originX, originY - 1, slow + originZ))
        column |= (1ULL << 0);

      if (tree->get(slow + originX, fast + originY, originZ + 32))
        layer |= (1ULL << 33);

      if (tree->get(slow + originX, fast + originY, originZ - 1))
        layer |= (1ULL << 0);
    }

    glm::vec3 coord = {originX / chunkSize, originY / chunkSize, originZ / chunkSize};

    /**
     * Cull meshing, ~0.13ms slower than greedy meshing
     *
     * CullMesh(coord, vertices, columns, rows, layers, chunkSize);
     */

    uint32_t widthStart[maskLength] = {};
    uint32_t heightStart[maskLength] = {};

    uint32_t widthEnd[maskLength] = {};
    uint32_t heightEnd[maskLength] = {};

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
    PrepareWidthHeightMasks(columns, widthStart, heightStart, widthEnd, heightEnd, chunkSize);

    /**
     * Greedy mesh the given axis
     */
    GreedyMeshAxis(coord, columns, widthStart, heightStart, widthEnd, heightEnd, vertices, FaceType::BOTTOM, FaceType::TOP, chunkSize);

    std::memset(widthStart, 0, sizeof(widthStart));
    std::memset(heightStart, 0, sizeof(heightStart));
    std::memset(widthEnd, 0, sizeof(widthEnd));
    std::memset(heightEnd, 0, sizeof(heightEnd));

    PrepareWidthHeightMasks(rows, widthStart, heightStart, widthEnd, heightEnd, chunkSize);
    GreedyMeshAxis(coord, rows, widthStart, heightStart, widthEnd, heightEnd, vertices, FaceType::LEFT, FaceType::RIGHT, chunkSize);

    std::memset(widthStart, 0, sizeof(widthStart));
    std::memset(heightStart, 0, sizeof(heightStart));
    std::memset(widthEnd, 0, sizeof(widthEnd));
    std::memset(heightEnd, 0, sizeof(heightEnd));

    PrepareWidthHeightMasks(layers, widthStart, heightStart, widthEnd, heightEnd, chunkSize);
    GreedyMeshAxis(coord, layers, widthStart, heightStart, widthEnd, heightEnd, vertices, FaceType::FRONT, FaceType::BACK, chunkSize);
  }
};