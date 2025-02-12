#pragma once

#include <vector>
#include <unordered_map>
#include <execution>

#include <glm/glm.hpp>

#include "Voxel.h"
#include "GreedyMesh.h"
#include "Utility.h"
#include "Engine/Types.h"
#include "Debug.h"

namespace Voxel
{
  class Manager
  {
  public:
    static constexpr const uint32_t Chunks = 8;

  private:
    std::unordered_map<glm::ivec3, Chunk, IVec3Hash> chunks;

  public:
    void set(const glm::ivec3 &position, const Type &type)
    {
      chunks[{std::floor(position.x / Chunk::ChunkSize), std::floor(position.y / Chunk::ChunkSize), std::floor(position.z / Chunk::ChunkSize)}].set({position.x % Chunk::ChunkSize, position.y % Chunk::ChunkSize, position.z % Chunk::ChunkSize}, Type::GRASS);
    }

    void clear()
    {
      chunks.clear();
    }

    void clear(int x, int y, int z)
    {
      chunks[{x, y, z}].clear();
    }

    void singleThread(std::vector<Vertex> &vertices)
    {

    }

    void greedyMesh(std::vector<Vertex> &vertices)
    {
      vertices.clear();

      // std::thread t([this, vertices]()
      //               { BENCHMARK("greedyMesh()", [this, vertices]() mutable
      //                           { multiThread(vertices); }, 100); });

      // t.detach();

      for (auto &[coord, chunk] : chunks)
      {
        std::vector<Chunk *> neighbours = {
            chunks.contains(coord + glm::ivec3(1, 0, 0)) ? &chunks.at(coord + glm::ivec3(1, 0, 0)) : nullptr,   // Right
            chunks.contains(coord + glm::ivec3(-1, 0, 0)) ? &chunks.at(coord + glm::ivec3(-1, 0, 0)) : nullptr, // Left
            chunks.contains(coord + glm::ivec3(0, 1, 0)) ? &chunks.at(coord + glm::ivec3(0, 1, 0)) : nullptr,   // Top
            chunks.contains(coord + glm::ivec3(0, -1, 0)) ? &chunks.at(coord + glm::ivec3(0, -1, 0)) : nullptr, // Bottom
            chunks.contains(coord + glm::ivec3(0, 0, 1)) ? &chunks.at(coord + glm::ivec3(0, 0, 1)) : nullptr,   // Front
            chunks.contains(coord + glm::ivec3(0, 0, -1)) ? &chunks.at(coord + glm::ivec3(0, 0, -1)) : nullptr  // Back
        };
        GreedyMesh::Chunk(coord, chunk, vertices, neighbours);
      }
    }
  };
}
