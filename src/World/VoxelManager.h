#pragma once

#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>

#include <algorithm>
#include <execution>

#include "Voxel.h"
#include "GreedyMesh.h"
#include "Utility.h"
#include "Engine/Types.h"
#include "Debug.h"

#include <fstream>

namespace Voxel
{
  // class Manager
  // {
  // public:
  //   static constexpr const uint32_t Chunks = 2;

  // private:
  //   std::vector<Chunk> chunks{Chunks * Chunks * Chunks};

  //   int index(int x, int y, int z)
  //   {
  //     return x + (Chunks * (y + (Chunks * z)));
  //   }

  // public:
  //   void set(const glm::ivec3 &position, const Type &type)
  //   {
  //     const int cx = static_cast<int>(std::floor(position.x / Chunk::ChunkSize));
  //     const int cy = static_cast<int>(std::floor(position.y / Chunk::ChunkSize));
  //     const int cz = static_cast<int>(std::floor(position.z / Chunk::ChunkSize));

  //     chunks[index(cx, cy, cz)].set({position.x % Chunk::ChunkSize, position.y % Chunk::ChunkSize, position.z % Chunk::ChunkSize}, type);
  //   }

  //   void clear()
  //   {
  //     chunks.clear();
  //     chunks.resize(Chunks * Chunks * Chunks);
  //   }

  //   void clear(int x, int y, int z)
  //   {
  //     chunks[index(x, y, z)].clear();
  //   }

  //   void greedyMesh(std::vector<Vertex> &vertices)
  //   {
  //     vertices.clear();

  //     // std::thread t([this, vertices]()
  //     //               { BENCHMARK("greedyMesh()", [this, vertices]() mutable
  //     //                           { multiThread(vertices); }, 100); });

  //     // t.detach();

  //     std::cout << "chunks.size(): " << chunks.size() << std::endl;
  //     for (size_t i = 0; i < chunks.size(); i++)
  //     {
  //       Chunk &chunk = chunks.at(i);

  //       glm::ivec3 coord = {std::floor(i % Chunks), std::floor((i / Chunks) % Chunks), std::floor(i / (Chunks * Chunks))};

  //       int right = i + 1;
  //       int left = i - 1;
  //       int top = i + Chunks;
  //       int bottom = i - Chunks;
  //       int front = i + (Chunks * Chunks);
  //       int back = i - (Chunks * Chunks);

  //       std::cout << coord.x << " " << coord.y << " " << coord.z << std::endl;

  //       std::vector<Chunk *> neighbours = {
  //           (right >= 0 && right < chunks.size()) ? &chunks[right] : nullptr,
  //           (left >= 0 && left < chunks.size()) ? &chunks[left] : nullptr,
  //           (top >= 0 && top < chunks.size()) ? &chunks[top] : nullptr,
  //           (bottom >= 0 && bottom < chunks.size()) ? &chunks[bottom] : nullptr,
  //           (front >= 0 && front < chunks.size()) ? &chunks[front] : nullptr,
  //           (back >= 0 && back < chunks.size()) ? &chunks[back] : nullptr,
  //       };

  //       // auto start1 = std::chrono::high_resolution_clock::now();

  //       GreedyMesh::Chunk(coord, chunk, vertices, neighbours);

  //       // auto end1 = std::chrono::high_resolution_clock::now();
  //       // std::chrono::duration<double> duration1 = end1 - start1;
  //       // std::cout << "GreedyMesh::Chunk took: " << duration1.count() << " seconds\n";
  //     }
  //   }
  // };

  class Manager
  {
  public:
    static constexpr const uint32_t Chunks = 2;

  private:
    std::unordered_map<glm::ivec3, Chunk, IVec3Hash> chunks;
    std::vector<Chunk> lchunks{Chunks * Chunks * Chunks};

    int index(int x, int y, int z)
    {
      return x + (Chunks * (y + (Chunks * z)));
    }

    glm::ivec3 position(int index)
    {
      return glm::ivec3{index / (Chunks * Chunks), (index / Chunks) % Chunks, index % Chunks};
    }

  public:
    void set(const glm::ivec3 &position, const Type &type)
    {
      const int cx = static_cast<int>(position.x / Chunk::ChunkSize);
      const int cy = static_cast<int>(position.y / Chunk::ChunkSize);
      const int cz = static_cast<int>(position.z / Chunk::ChunkSize);

      lchunks[index(cx, cy, cz)].set({position.x % Chunk::ChunkSize, position.y % Chunk::ChunkSize, position.z % Chunk::ChunkSize}, type);

      chunks[{cx, cy, cz}].set({position.x % Chunk::ChunkSize, position.y % Chunk::ChunkSize, position.z % Chunk::ChunkSize}, Type::GRASS);
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

        int i = index(coord.x, coord.y, coord.z);

        // int right = i + 1;
        // int left = i - 1;
        // int top = i + Chunks;
        // int bottom = i - Chunks;
        // int front = i + (Chunks * Chunks);
        // int back = i - (Chunks * Chunks);

        int right = (coord.x + 1 >= Chunks) ? -1 : i + 1;
        int left = (coord.x - 1 < 0) ? -1 : i - 1;
        int top = (coord.y + 1 >= Chunks) ? -1 : i + Chunks;
        int bottom = (coord.y - 1 < 0) ? -1 : i - Chunks;
        int front = (coord.z + 1 >= Chunks) ? -1 : i + (Chunks * Chunks);
        int back = (coord.z - 1 < 0) ? -1 : i - (Chunks * Chunks);

        glm::ivec3 icoord = position(i);

        std::cout << coord.x << " " << coord.y << " " << coord.z << std::endl;
        std::cout << right << " " << lchunks.size() << " " << icoord.x << " " << icoord.y << " " << icoord.z << std::endl;

        std::vector<Chunk *> lneighbours = {
            (right > -1) ? &lchunks[right] : nullptr,
            (left >= 0 && left < lchunks.size()) ? &lchunks[left] : nullptr,
            (top >= 0 && top < lchunks.size()) ? &lchunks[top] : nullptr,
            (bottom >= 0 && bottom < lchunks.size()) ? &lchunks[bottom] : nullptr,
            (front >= 0 && front < lchunks.size()) ? &lchunks[front] : nullptr,
            (back >= 0 && back < lchunks.size()) ? &lchunks[back] : nullptr,
        };

        std::cout << neighbours[0] << " " << lneighbours[0] << std::endl;

        // auto start1 = std::chrono::high_resolution_clock::now();

        // GreedyMesh::Chunk(coord, chunk, vertices, neighbours);
        GreedyMesh::Chunk(coord, chunk, vertices, lneighbours);

        // auto end1 = std::chrono::high_resolution_clock::now();
        // std::chrono::duration<double> duration1 = end1 - start1;
        // std::cout << "GreedyMesh::Chunk took: " << duration1.count() << " seconds\n";
      }
    }
  };
}
