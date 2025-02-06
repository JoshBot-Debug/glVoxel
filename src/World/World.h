#pragma once

#include <algorithm>

#include "Engine/Core/Buffer.h"
#include "Engine/Core/VertexArray.h"
#include "Engine/Model.h"
// #include "World/Chunk.h"
// #include "World/ChunkManager.h"
#include "World/VoxelManager.h"

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
  Buffer vbo;
  VertexArray vao;
  Voxel::Manager voxels;
  std::vector<Vertex> vertices;

public:
  World() : vbo(BufferTarget::ARRAY_BUFFER, VertexDraw::DYNAMIC)
  {
    vao.generate();
    vbo.generate();
  }

  void draw()
  {
    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glDrawArrays(GL_LINES, 0, vertices.size());
  }

  void setBuffer()
  {
    vao.bind();
    vbo.set(vertices);
    vao.set(0, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)(offsetof(Vertex, x)));
    vao.set(1, 1, VertexType::FLOAT, false, sizeof(Vertex), (void *)(offsetof(Vertex, normal)));
  }

  void generateTerrain()
  {
    voxels.clear();

    // fill();
    // fillSphere();
    // fillAlternate();

    noise::module::Perlin perlin;
    perlin.SetSeed(static_cast<int>(std::time(0)));

    utils::NoiseMap heightMap;
    utils::NoiseMapBuilderPlane heightMapBuilder;

    heightMapBuilder.SetSourceModule(perlin);
    heightMapBuilder.SetDestNoiseMap(heightMap);
    heightMapBuilder.SetDestSize(Voxel::Chunk::SIZE * Voxel::Manager::CHUNKS, Voxel::Chunk::SIZE * Voxel::Manager::CHUNKS);
    heightMapBuilder.SetBounds(1.0, 2.0, 1.0, 2.0);
    heightMapBuilder.Build();

    for (int z = 0; z < Voxel::Chunk::SIZE * Voxel::Manager::CHUNKS; ++z)
    {
      for (int x = 0; x < Voxel::Chunk::SIZE * Voxel::Manager::CHUNKS; ++x)
      {
        float n = heightMap.GetValue(x, z);
        unsigned int height = static_cast<unsigned int>(std::round((std::clamp(n, -1.0f, 1.0f) + 1) * ((Voxel::Chunk::SIZE * Voxel::Manager::CHUNKS) / 2)));
        std::cout << z << " " << x << " " << height << std::endl;
        for (size_t y = 0; y < height; y++)
          voxels.set({x, y, z}, Voxel::Type::GRASS);
      }
    }

    voxels.greedyMesh(vertices);
    setBuffer();
  }

  void fillAlternate()
  {
    const int size = Voxel::Chunk::SIZE * Voxel::Manager::CHUNKS;
    for (size_t z = 0; z < size; z += 2)
      for (size_t x = 0; x < size; x += 2)
        for (size_t y = 0; y < size; y += 2)
          voxels.set({x, y, z}, Voxel::Type::GRASS);
  }

  void fill()
  {
    const int size = Voxel::Chunk::SIZE * Voxel::Manager::CHUNKS;
    for (size_t z = 0; z < size; z++)
      for (size_t x = 0; x < size; x++)
        for (size_t y = 0; y < size; y++)
          voxels.set({x, y, z}, Voxel::Type::GRASS);
  }

  void fillSphere()
  {
    const unsigned int size = Voxel::Chunk::SIZE * Voxel::Manager::CHUNKS;
    const glm::ivec3 center(size / 2);
    int radius = std::min({center.x, center.y, center.z}) - 1.0f;

    for (size_t z = 0; z < size; z++)
      for (size_t x = 0; x < size; x++)
        for (size_t y = 0; y < size; y++)
        {
          int dx = x - center.x;
          int dy = y - center.y;
          int dz = z - center.z;

          int distance = std::sqrt(dx * dx + dy * dy + dz * dz);

          if (distance <= radius)
            voxels.set({x, y, z}, Voxel::Type::GRASS);
        }
  }
};