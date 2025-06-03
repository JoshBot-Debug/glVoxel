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
  //   std::ofstream logfile{"log.txt", std::ios::app};

  //   std::vector<Chunk> chunks{Chunks * Chunks * Chunks};

  //   int index(int x, int y, int z)
  //   {
  //     return x + (Chunks * (y + (Chunks * z)));
  //   }

  // public:
  //   void set(const glm::ivec3 &position, const Type &type)
  //   {
  //     // const int cx = static_cast<int>(std::floor(position.x / Chunk::ChunkSize));
  //     // const int cy = static_cast<int>(std::floor(position.y / Chunk::ChunkSize));
  //     // const int cz = static_cast<int>(std::floor(position.z / Chunk::ChunkSize));

  //     if (logfile.is_open())
  //     {
  //       logfile << "i: " << index(cx, cy, cz) << " ";
  //       logfile << "p: " << position.x << " " << position.y << " " << position.z << " ";
  //       logfile << "c: " << cx << " " << cy << " " << cz << std::endl;
  //     }

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

  //       // Top
  //       // 2[0,1,0] 3[1,1,0]
  //       // 6[0,1,1] 7[1,1,1]

  //       // Bottom
  //       // 0[0,0,0] 1[1,0,0]
  //       // 4[0,0,1] 5[1,0,1]

  //       // In Order
  //       // 0[0,0,0] 1[1,0,0]
  //       // 2[0,1,0] 3[1,1,0]
  //       // 4[0,0,1] 5[1,0,1]
  //       // 6[0,1,1] 7[1,1,1]

  //       glm::ivec3 coord = {i % Chunks, std::floor((i / Chunks) % Chunks), std::floor(i / (Chunks * Chunks))};

  //       int right = index(coord.x + 1, coord.y, coord.z);
  //       int left = index(coord.x - 1, coord.y, coord.z);
  //       int top = index(coord.x, coord.y + 1, coord.z);
  //       int bottom = index(coord.x, coord.y - 1, coord.z);
  //       int front = index(coord.x, coord.y, coord.z + 1);
  //       int back = index(coord.x, coord.y, coord.z - 1);

  //       std::cout << index(coord.x, coord.y, coord.z) << " " << coord.x << " " << coord.y << " " << coord.z << std::endl;

  //       std::vector<Chunk *> neighbours = {
  //           (right >= 0 && right < chunks.size()) ? &chunks[right] : nullptr,
  //           (left >= 0 && left < chunks.size()) ? &chunks[left] : nullptr,
  //           (top >= 0 && top < chunks.size()) ? &chunks[top] : nullptr,
  //           (bottom >= 0 && bottom < chunks.size()) ? &chunks[bottom] : nullptr,
  //           (front >= 0 && front < chunks.size()) ? &chunks[front] : nullptr,
  //           (back >= 0 && back < chunks.size()) ? &chunks[back] : nullptr,
  //       };

  //       // std::cout << "right: " << right << " n:" << neighbours[0] << std::endl;
  //       // std::cout << "left: " << left << " n:" << neighbours[1] << std::endl;
  //       // std::cout << "top: " << top << " n:" << neighbours[2] << std::endl;
  //       // std::cout << "bottom: " << bottom << " n:" << neighbours[3] << std::endl;
  //       // std::cout << "front: " << front << " n:" << neighbours[4] << std::endl;
  //       // std::cout << "back: " << back << " n:" << neighbours[5] << std::endl;

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
    // std::unordered_map<glm::ivec3, Chunk, IVec3Hash> chunks;
    std::unordered_map<int, Chunk> chunks;

    int index(int x, int y, int z)
    {
      return x + (Chunks * (y + (Chunks * z)));
    }

  public:
    void set(const glm::ivec3 &position, const Type &type)
    {
      int i = index(position.x / Chunk::ChunkSize, position.y / Chunk::ChunkSize, position.z / Chunk::ChunkSize);
      // chunks[{std::floor(position.x / Chunk::ChunkSize), std::floor(position.y / Chunk::ChunkSize), std::floor(position.z / Chunk::ChunkSize)}].set({position.x % Chunk::ChunkSize, position.y % Chunk::ChunkSize, position.z % Chunk::ChunkSize}, Type::GRASS);
      chunks[i].set({position.x % Chunk::ChunkSize, position.y % Chunk::ChunkSize, position.z % Chunk::ChunkSize}, Type::GRASS);
    }

    void clear()
    {
      chunks.clear();
    }

    void clear(int x, int y, int z)
    {
      int i = index(x, y, z);
      // chunks[{x, y, z}].clear();
      chunks[i].clear();
    }

    void greedyMesh(std::vector<Vertex> &vertices)
    {
      vertices.clear();

      // std::thread t([this, vertices]()
      //               { BENCHMARK("greedyMesh()", [this, vertices]() mutable
      //                           { multiThread(vertices); }, 100); });

      // t.detach();

      for (auto &[i, chunk] : chunks)
      {
        glm::ivec3 coord = {std::floor(i % Chunks), std::floor((i / Chunks) % Chunks), std::floor(i / (Chunks * Chunks))};

        int right = index(coord.x + 1, coord.y, coord.z);
        int left = index(coord.x - 1, coord.y, coord.z);
        int top = index(coord.x, coord.y + 1, coord.z);
        int bottom = index(coord.x, coord.y - 1, coord.z);
        int front = index(coord.x, coord.y, coord.z + 1);
        int back = index(coord.x, coord.y, coord.z - 1);

        std::vector<Chunk *> neighbours = {
            chunks.contains(right) ? &chunks.at(right) : nullptr,   // Right
            chunks.contains(left) ? &chunks.at(left) : nullptr,     // Left
            chunks.contains(top) ? &chunks.at(top) : nullptr,       // Top
            chunks.contains(bottom) ? &chunks.at(bottom) : nullptr, // Bottom
            chunks.contains(front) ? &chunks.at(front) : nullptr,   // Front
            chunks.contains(back) ? &chunks.at(back) : nullptr      // Back
        };

        // std::vector<Chunk *> neighbours = {
        //     chunks.contains(coord + glm::ivec3(1, 0, 0)) ? &chunks.at(coord + glm::ivec3(1, 0, 0)) : nullptr,   // Right
        //     chunks.contains(coord + glm::ivec3(-1, 0, 0)) ? &chunks.at(coord + glm::ivec3(-1, 0, 0)) : nullptr, // Left
        //     chunks.contains(coord + glm::ivec3(0, 1, 0)) ? &chunks.at(coord + glm::ivec3(0, 1, 0)) : nullptr,   // Top
        //     chunks.contains(coord + glm::ivec3(0, -1, 0)) ? &chunks.at(coord + glm::ivec3(0, -1, 0)) : nullptr, // Bottom
        //     chunks.contains(coord + glm::ivec3(0, 0, 1)) ? &chunks.at(coord + glm::ivec3(0, 0, 1)) : nullptr,   // Front
        //     chunks.contains(coord + glm::ivec3(0, 0, -1)) ? &chunks.at(coord + glm::ivec3(0, 0, -1)) : nullptr  // Back
        // };

        auto start1 = std::chrono::high_resolution_clock::now();

        GreedyMesh::Chunk(coord, chunk, vertices, neighbours);

        auto end1 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration1 = end1 - start1;
        std::cout << "GreedyMesh::Chunk took: " << duration1.count() << " seconds\n";
      }
    }
  };
}
