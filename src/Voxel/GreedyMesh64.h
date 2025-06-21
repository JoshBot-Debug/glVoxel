#pragma once

#include <bitset>
#include <cstring>
#include <vector>

#include <glm/glm.hpp>

#include "Engine/Face.h"
#include "Engine/Types.h"
#include "Voxel/SparseVoxelOctree.h"

#include <immintrin.h>

class GreedyMesh64 {
public:
  static constexpr uint8_t CHUNK_SIZE = 64;
  static constexpr unsigned int MASK_LENGTH = CHUNK_SIZE * CHUNK_SIZE;

private:
  static uint64_t ClearLowestBits(uint64_t bits, int n) {
    return (n >= CHUNK_SIZE) ? 0 : (bits & ~((1ULL << n) - 1));
  }

  static void SetWidthHeight(uint8_t a, uint8_t b, uint64_t bits,
                             uint64_t (&widthMasks)[],
                             uint64_t (&heightMasks)[]);

  static void
  PrepareWidthHeightMasks(const uint64_t (&bits)[], uint8_t paddingIndex,
                          uint8_t (&padding)[], uint64_t (&widthStart)[],
                          uint64_t (&heightStart)[], uint64_t (&widthEnd)[],
                          uint64_t (&heightEnd)[]);

  static void GreedyMesh64Face(const glm::ivec3 &offsetPosition, uint8_t a,
                               uint8_t b, uint64_t bits,
                               uint64_t (&widthMasks)[],
                               uint64_t (&heightMasks)[],
                               std::vector<Vertex> &vertices, FaceType type);

  static void GreedyMesh64Axis(const glm::ivec3 &offsetPosition,
                               const uint64_t (&bits)[],
                               uint64_t (&widthStart)[],
                               uint64_t (&heightStart)[],
                               uint64_t (&widthEnd)[], uint64_t (&heightEnd)[],
                               std::vector<Vertex> &vertices,
                               FaceType startType, FaceType endType);

  static void CullMesh(const glm::ivec3 &offsetPosition,
                       std::vector<Vertex> &vertices, uint64_t (&columns)[],
                       uint64_t (&rows)[], uint64_t (&layers)[]);

public:
  static void Octree(SparseVoxelOctree *tree, std::vector<Vertex> &vertices,
                     int originX, int originY, int originZ, Voxel *filter = nullptr);
};