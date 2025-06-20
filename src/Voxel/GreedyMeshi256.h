#pragma once

#include <bitset>
#include <cstring>
#include <vector>

#include <glm/glm.hpp>

#include "Engine/Face.h"
#include "Engine/Types.h"
#include "Voxel/SparseVoxelOctree.h"

#include <immintrin.h>

class GreedyMeshi256 {
public:
  static constexpr uint8_t s_BITS = 64;
  static constexpr uint16_t s_CHUNK_SIZE = 256;
  static constexpr uint16_t s_STEPS = s_CHUNK_SIZE / s_BITS;
  static constexpr unsigned int s_MASK_LENGTH =
      (s_CHUNK_SIZE * s_CHUNK_SIZE) * s_STEPS;

private:
  static void SetWidthHeight(uint8_t a, uint8_t b, __m256i &bits,
                             uint64_t *widthMasks, uint64_t *heightMasks);

  static void PrepareWidthHeightMasks(uint64_t *bits, uint8_t paddingIndex,
                                      uint8_t *padding, uint64_t *widthStart,
                                      uint64_t *heightStart, uint64_t *widthEnd,
                                      uint64_t *heightEnd);

  static void GreedyMeshi256Face(const glm::ivec3 &offsetPosition, uint8_t a,
                                 uint8_t b, __m256i &bits, uint64_t *widthMasks,
                                 uint64_t *heightMasks,
                                 std::vector<Vertex> &vertices, FaceType type);

  static void GreedyMeshi256Axis(const glm::ivec3 &offsetPosition,
                                 uint64_t *bits, uint64_t *widthStart,
                                 uint64_t *heightStart, uint64_t *widthEnd,
                                 uint64_t *heightEnd,
                                 std::vector<Vertex> &vertices,
                                 FaceType startType, FaceType endType);

public:
  static void Octree(SparseVoxelOctree *tree, std::vector<Vertex> &vertices,
                     int originX, int originY, int originZ, int depth,
                     Voxel *filter = nullptr);

  /**
   * Clear lowest n bits
   */
  static __m256i clb256(__m256i &bits, int n);
  static int clz256(uint64_t *bits);
  static int clz256(__m256i &bits);
  static int ctz256(uint64_t *bits);
  static int ctz256(__m256i &bits);
  static __m256i sl256(__m256i &bits, int n);
  static __m256i sr256(__m256i &bits, int n);
  static int ffs256(__m256i &bits);

  /**
   * Makes a bitmask range with an offset
   * int size = 4;
   * int offset = 2;
   * 1ULL => 0b00000001
   * 1ULL << size => 0b00010000
   * (1ULL << size) -1 => 0b00001111
   * ((1ULL << size) -1) << offset => 0b00111100
   */
  static __m256i mmr(int size, int offset);
};