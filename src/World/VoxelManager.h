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

namespace Voxel
{
  class Manager
  {
  public:
    static constexpr const uint32_t Chunks = 2;

  private:
    std::vector<Chunk> chunks{Chunks * Chunks * Chunks};

    int index(int x, int y, int z)
    {
      return x + (Chunks * (y + (Chunks * z)));
    }

  public:
    void set(const glm::ivec3 &position, const Type &type)
    {
      const int cx = static_cast<int>(std::floor(position.x / Chunk::ChunkSize));
      const int cy = static_cast<int>(std::floor(position.y / Chunk::ChunkSize));
      const int cz = static_cast<int>(std::floor(position.z / Chunk::ChunkSize));

      chunks[index(cx, cy, cz)].set({position.x % Chunk::ChunkSize, position.y % Chunk::ChunkSize, position.z % Chunk::ChunkSize}, type);
    }

    void clear()
    {
      chunks.clear();
      chunks.resize(Chunks * Chunks * Chunks);
    }

    void clear(int x, int y, int z)
    {
      chunks[index(x, y, z)].clear();
    }

    void greedyMesh(std::vector<Vertex> &vertices)
    {
      vertices.clear();

      // std::thread t([this, vertices]()
      //               { BENCHMARK("greedyMesh()", [this, vertices]() mutable
      //                           { multiThread(vertices); }, 100); });

      // t.detach();

      std::cout << "chunks.size(): " << chunks.size() << std::endl;
      for (size_t i = 0; i < chunks.size(); i++)
      {
        Chunk &chunk = chunks.at(i);

        // Top
        // 2[0,1,0] 3[1,1,0]
        // 6[0,1,1] 7[1,1,1]

        // Bottom
        // 0[0,0,0] 1[1,0,0]
        // 4[0,0,1] 5[1,0,1]

        // In Order
        // 0[0,0,0] 1[1,0,0]
        // 2[0,1,0] 3[1,1,0]
        // 4[0,0,1] 5[1,0,1]
        // 6[0,1,1] 7[1,1,1]

        glm::ivec3 coord = {i % Chunks, (i / Chunks) % Chunks, i / (Chunks * Chunks)};

        int right = (coord.x + 1) + (Chunks * (coord.y + (Chunks * coord.z)));
        int left = (coord.x - 1) + (Chunks * (coord.y + (Chunks * coord.z)));
        int top = coord.x + (Chunks * ((coord.y + 1) + (Chunks * coord.z)));
        int bottom = coord.x + (Chunks * ((coord.y - 1) + (Chunks * coord.z)));
        int front = coord.x + (Chunks * (coord.y + (Chunks * (coord.z + 1))));
        int back = coord.x + (Chunks * (coord.y + (Chunks * (coord.z - 1))));

        std::cout << index(coord.x, coord.y, coord.z) << " " << coord.x << " " << coord.y << " " << coord.z << std::endl;

        std::vector<Chunk *> neighbours = {
            (right >= 0 && right < chunks.size()) ? &chunks[right] : nullptr,
            (left >= 0 && left < chunks.size()) ? &chunks[left] : nullptr,
            (top >= 0 && top < chunks.size()) ? &chunks[top] : nullptr,
            (bottom >= 0 && bottom < chunks.size()) ? &chunks[bottom] : nullptr,
            (front >= 0 && front < chunks.size()) ? &chunks[front] : nullptr,
            (back >= 0 && back < chunks.size()) ? &chunks[back] : nullptr,
        };

        // std::cout << "right: " << right << " n:" << neighbours[0] << std::endl;
        // std::cout << "left: " << left << " n:" << neighbours[1] << std::endl;
        // std::cout << "top: " << top << " n:" << neighbours[2] << std::endl;
        // std::cout << "bottom: " << bottom << " n:" << neighbours[3] << std::endl;
        // std::cout << "front: " << front << " n:" << neighbours[4] << std::endl;
        // std::cout << "back: " << back << " n:" << neighbours[5] << std::endl;

        // auto start1 = std::chrono::high_resolution_clock::now();

        GreedyMesh::Chunk(coord, chunk, vertices, neighbours);

        // auto end1 = std::chrono::high_resolution_clock::now();
        // std::chrono::duration<double> duration1 = end1 - start1;
        // std::cout << "GreedyMesh::Chunk took: " << duration1.count() << " seconds\n";
      }
    }
  };

  // class Manager
  // {
  // public:
  //   static constexpr const uint32_t Chunks = 16;

  // private:
  //   std::unordered_map<glm::ivec3, Chunk, IVec3Hash> chunks;

  // public:
  //   void set(const glm::ivec3 &position, const Type &type)
  //   {
  //     chunks[{std::floor(position.x / Chunk::ChunkSize), std::floor(position.y / Chunk::ChunkSize), std::floor(position.z / Chunk::ChunkSize)}].set({position.x % Chunk::ChunkSize, position.y % Chunk::ChunkSize, position.z % Chunk::ChunkSize}, Type::GRASS);
  //   }

  //   void clear()
  //   {
  //     chunks.clear();
  //   }

  //   void clear(int x, int y, int z)
  //   {
  //     chunks[{x, y, z}].clear();
  //   }

  //   void greedyMesh(std::vector<Vertex> &vertices)
  //   {
  //     vertices.clear();

  //     // std::thread t([this, vertices]()
  //     //               { BENCHMARK("greedyMesh()", [this, vertices]() mutable
  //     //                           { multiThread(vertices); }, 100); });

  //     // t.detach();

  //     for (auto &[coord, chunk] : chunks)
  //     {
  //       std::vector<Chunk *> neighbours = {
  //           chunks.contains(coord + glm::ivec3(1, 0, 0)) ? &chunks.at(coord + glm::ivec3(1, 0, 0)) : nullptr,   // Right
  //           chunks.contains(coord + glm::ivec3(-1, 0, 0)) ? &chunks.at(coord + glm::ivec3(-1, 0, 0)) : nullptr, // Left
  //           chunks.contains(coord + glm::ivec3(0, 1, 0)) ? &chunks.at(coord + glm::ivec3(0, 1, 0)) : nullptr,   // Top
  //           chunks.contains(coord + glm::ivec3(0, -1, 0)) ? &chunks.at(coord + glm::ivec3(0, -1, 0)) : nullptr, // Bottom
  //           chunks.contains(coord + glm::ivec3(0, 0, 1)) ? &chunks.at(coord + glm::ivec3(0, 0, 1)) : nullptr,   // Front
  //           chunks.contains(coord + glm::ivec3(0, 0, -1)) ? &chunks.at(coord + glm::ivec3(0, 0, -1)) : nullptr  // Back
  //       };

  //       auto start1 = std::chrono::high_resolution_clock::now();

  //       GreedyMesh::Chunk(coord, chunk, vertices, neighbours);

  //       auto end1 = std::chrono::high_resolution_clock::now();
  //       std::chrono::duration<double> duration1 = end1 - start1;
  //       std::cout << "GreedyMesh::Chunk took: " << duration1.count() << " seconds\n";
  //     }
  //   }
  // };
}
