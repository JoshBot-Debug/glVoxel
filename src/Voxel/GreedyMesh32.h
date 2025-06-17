#pragma once

#include <bitset>
#include <cstring>
#include <vector>

#include <glm/glm.hpp>

#include "Engine/Face.h"
#include "Engine/Types.h"
#include "Voxel/SparseVoxelOctree.h"

#include <immintrin.h>

class GreedyMesh32 {
public:
  static constexpr uint8_t CHUNK_SIZE = 32;

private:
  static uint32_t ClearLowestBits(uint32_t bits, int n) {
    return (n >= CHUNK_SIZE) ? 0 : (bits & ~((1U << n) - 1));
  }

  static void SetWidthHeight(uint8_t a, uint8_t b, uint32_t bits,
                             uint32_t (&widthMasks)[],
                             uint32_t (&heightMasks)[]);

  static void
  PrepareWidthHeightMasks(const uint32_t (&bits)[], uint8_t paddingIndex,
                          uint8_t (&padding)[], uint32_t (&widthStart)[],
                          uint32_t (&heightStart)[], uint32_t (&widthEnd)[],
                          uint32_t (&heightEnd)[]);

  static void GreedyMesh32Face(const glm::ivec3 &offsetPosition, uint8_t a,
                             uint8_t b, uint32_t bits, uint32_t (&widthMasks)[],
                             uint32_t (&heightMasks)[],
                             std::vector<Vertex> &vertices, FaceType type);

  static void GreedyMesh32Axis(const glm::ivec3 &offsetPosition,
                             const uint32_t (&bits)[], uint32_t (&widthStart)[],
                             uint32_t (&heightStart)[], uint32_t (&widthEnd)[],
                             uint32_t (&heightEnd)[],
                             std::vector<Vertex> &vertices, FaceType startType,
                             FaceType endType);

  static void CullMesh(const glm::ivec3 &offsetPosition,
                       std::vector<Vertex> &vertices, uint32_t (&columns)[],
                       uint32_t (&rows)[], uint32_t (&layers)[]);

public:
  static void Octree(SparseVoxelOctree *tree, std::vector<Vertex> &vertices,
                     int originX, int originY, int originZ, int depth,
                     Voxel *filter = nullptr);
};