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
  static void SetWidthHeight(unsigned int a, unsigned int b, uint32_t bits, uint32_t (&widthMasks)[], uint32_t (&heightMasks)[], unsigned int chunkSize);

  static void PrepareWidthHeightMasks(const uint64_t (&bits)[], uint32_t (&widthStart)[], uint32_t (&heightStart)[], uint32_t (&widthEnd)[], uint32_t (&heightEnd)[], unsigned int chunkSize);

  static void GreedyMeshFace(const glm::ivec3 &offsetPosition, uint8_t a, uint8_t b, uint64_t bits, uint32_t (&widthMasks)[], uint32_t (&heightMasks)[], std::vector<Vertex> &vertices, FaceType type, unsigned int chunkSize);

  static void GreedyMeshAxis(const glm::ivec3 &offsetPosition, const uint64_t (&bits)[], uint32_t (&widthStart)[], uint32_t (&heightStart)[], uint32_t (&widthEnd)[], uint32_t (&heightEnd)[], std::vector<Vertex> &vertices, FaceType startType, FaceType endType, unsigned int chunkSize);

  static void CullMesh(const glm::ivec3 &offsetPosition, std::vector<Vertex> &vertices, uint64_t (&columns)[], uint64_t (&rows)[], uint64_t (&layers)[], unsigned int chunkSize);

public:
  static void SparseVoxelTree(Voxel::SparseVoxelOctree *tree, std::vector<Vertex> &vertices, int originX, int originY, int originZ, unsigned int chunkSize, unsigned int maskLength);
};