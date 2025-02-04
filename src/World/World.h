#pragma once

#include <algorithm>

#include "Engine/Core/Buffer.h"
#include "Engine/Core/VertexArray.h"
#include "Engine/Model.h"
#include "World/Voxel.h"
#include "World/VoxelGrid.h"

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

  VoxelGrid grid;
  static constexpr unsigned int CHUNK_SIZE = 32;
  // std::unordered_map<glm::ivec3, VoxelGrid, IVec3Hash> chunks;
  std::vector<Vertex> vertices;

public:
  World() : vbo(BufferTarget::ARRAY_BUFFER, VertexDraw::DYNAMIC)
  {
    vao.generate();
    vbo.generate();

    // fillNoise({128, 128, 128});
    fillSphere(grid.size());
    // fillAlternate(grid.size());
    // fill(grid.size(), 1);

    update();
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
    grid.mesh(vertices);
    // for (auto &[coord, grid] : chunks)
    //   grid.mesh(vertices);
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
    fillNoise({0, 0, 0});
    update();
    setBuffer();
  }

  void fillNoise(const glm::ivec3 &size)
  {
    grid.clear();
    // for (auto &[coord, grid] : chunks)
    //   grid.clear();

    noise::module::Perlin perlin;
    perlin.SetSeed(static_cast<int>(std::time(0)));

    utils::NoiseMap heightMap;
    utils::NoiseMapBuilderPlane heightMapBuilder;

    heightMapBuilder.SetSourceModule(perlin);
    heightMapBuilder.SetDestNoiseMap(heightMap);
    heightMapBuilder.SetDestSize(size.z, size.x);
    heightMapBuilder.SetBounds(1.0, 2.0, 1.0, 2.0);
    heightMapBuilder.Build();

    for (int z = 0; z < size.z; ++z)
    {
      for (int x = 0; x < size.x; ++x)
      {
        float n = heightMap.GetValue(x, z);
        unsigned int height = static_cast<int>(std::round((15 * (std::clamp(n, -1.0f, 1.0f) + 1)))) + 1;
        for (size_t y = 0; y < height; y++)
          grid.set(x, y, z, 1);
        // chunks[{std::floor(x / CHUNK_SIZE), std::floor(y / CHUNK_SIZE), std::floor(z / CHUNK_SIZE)}].set(x % CHUNK_SIZE, y % CHUNK_SIZE, z % CHUNK_SIZE, 1);
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

  void fillSphere(const glm::ivec3 &size)
  {
    const glm::ivec3 center = size / 2;

    int radius = std::min({center.x, center.y, center.z}) - 1.0f;

    for (size_t z = 0; z < size.z; z++)
      for (size_t x = 0; x < size.x; x++)
        for (size_t y = 0; y < size.y; y++)
        {
          int dx = x - center.x;
          int dy = y - center.y;
          int dz = z - center.z;

          int distance = std::sqrt(dx * dx + dy * dy + dz * dz);

          if (distance <= radius)
            grid.set(x, y, z, 1);
        }
  }
};