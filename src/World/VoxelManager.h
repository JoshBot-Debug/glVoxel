#pragma once

#include <vector>
#include <unordered_map>

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
    static constexpr const uint32_t CHUNKS = 8;

  private:
    std::unordered_map<glm::ivec3, Chunk, IVec3Hash> chunks;

  public:
    void set(const glm::ivec3 &position, const Type &type)
    {
      chunks[{std::floor(position.x / Chunk::SIZE), std::floor(position.y / Chunk::SIZE), std::floor(position.z / Chunk::SIZE)}].set({position.x % Chunk::SIZE, position.y % Chunk::SIZE, position.z % Chunk::SIZE}, Type::GRASS);
    }

    void clear()
    {
      chunks.clear();
    }

    void clear(int x, int y, int z)
    {
      chunks[{x, y, z}].clear();
    }

    void greedyMesh(std::vector<Vertex> &vertices)
    {
      vertices.clear();

      std::thread t([this, vertices]()
                    { BENCHMARK("greedyMesh()", [this, vertices]() mutable
                                { for (auto &[coord, chunk] : this->chunks)
                                    GreedyMesh::Chunk(coord, chunk, vertices); }, 1000); });

      t.detach();

      for (auto &[coord, chunk] : chunks)
        GreedyMesh::Chunk(coord, chunk, vertices);
    }
  };
}
