#pragma once

#include <algorithm>

#include "Engine/Core/Buffer.h"
#include "Engine/Core/VertexArray.h"
#include "Engine/Model.h"
#include "World/VoxelChunk.h"
#include "World/ChunkManager.h"

#include <iostream>
#include <noise/noise.h>
#include <noise/noiseutils.h>
#include <ctime>
#include <bitset>

#include <chrono>
#include <thread>

class World
{
private:
  VertexArray vao;
  Buffer vbo;

  ChunkManager chunkManager;
  std::vector<Vertex> vertices;

public:
  World() : vbo(BufferTarget::ARRAY_BUFFER, VertexDraw::DYNAMIC)
  {
    vao.generate();
    vbo.generate();
    fillNoise();
    chunkManager.update(vertices);
    setBuffer();
  }

  void draw()
  {
    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glDrawArrays(GL_LINES, 0, vertices.size());
  }

  void update()
  {
    // chunkManager.update(vertices);
  }

  void setBuffer()
  {
    vao.bind();
    vbo.set(vertices);
    vao.set(0, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)(offsetof(Vertex, x)));
    vao.set(1, 1, VertexType::FLOAT, false, sizeof(Vertex), (void *)(offsetof(Vertex, normal)));
  }

  void generateNoise()
  {
    fillNoise();
    chunkManager.update(vertices);
    setBuffer();
  }

  void fillNoise()
  {
    chunkManager.clearAll();

    noise::module::Perlin perlin;
    perlin.SetSeed(static_cast<int>(std::time(0)));

    utils::NoiseMap heightMap;
    utils::NoiseMapBuilderPlane heightMapBuilder;

    heightMapBuilder.SetSourceModule(perlin);
    heightMapBuilder.SetDestNoiseMap(heightMap);
    // heightMapBuilder.SetDestSize(VoxelChunk::SIZE * ChunkManager::CHUNKS, VoxelChunk::SIZE * ChunkManager::CHUNKS);
    heightMapBuilder.SetDestSize(32,32);
    heightMapBuilder.SetBounds(1.0, 2.0, 1.0, 2.0);
    heightMapBuilder.Build();

    for (int z = 0; z < VoxelChunk::SIZE * ChunkManager::CHUNKS; ++z)
    {
      for (int x = 0; x < VoxelChunk::SIZE * ChunkManager::CHUNKS; ++x)
      {
        float n = heightMap.GetValue(x, z);
        unsigned int height = static_cast<unsigned int>(std::round((std::clamp(n, -1.0f, 1.0f) + 1) * 16));
        for (size_t y = 0; y < height; y++)
          chunkManager.set(x, y, z, 1);
      }
    }
  }

  // void fill(const glm::ivec3 &size, unsigned int value = 1)
  // {
  //   for (size_t z = 0; z < size.z; z++)
  //     for (size_t x = 0; x < size.x; x++)
  //       for (size_t y = 0; y < size.y; y++)
  //         grid.set(x, y, z, value);
  // }

  // void fillAlternate(const glm::ivec3 &size)
  // {
  //   for (size_t z = 0; z < size.z; z += 2)
  //     for (size_t x = 0; x < size.x; x += 2)
  //       for (size_t y = 0; y < size.y; y += 2)
  //         grid.set(x, y, z, 1);
  // }

  // void fillSphere(const glm::ivec3 &size)
  // {
  //   const glm::ivec3 center = size / 2;
  //   int radius = std::min({center.x, center.y, center.z}) - 1.0f;
  //   for (size_t z = 0; z < size.z; z++)
  //     for (size_t x = 0; x < size.x; x++)
  //       for (size_t y = 0; y < size.y; y++)
  //       {
  //         int dx = x - center.x;
  //         int dy = y - center.y;
  //         int dz = z - center.z;
  //         int distance = std::sqrt(dx * dx + dy * dy + dz * dz);
  //         if (distance <= radius)
  //           grid.set(x, y, z, 1);
  //       }
  // }
};