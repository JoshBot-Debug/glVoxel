#pragma once

#include <algorithm>

#include "Engine/Core/Buffer.h"
#include "Engine/Core/VertexArray.h"
#include "Engine/Model.h"
#include "World/VoxelManager.h"

#include <iostream>
#include <noise/noise.h>
#include <noise/noiseutils.h>
#include <ctime>
#include <bitset>

#include <chrono>
#include <thread>

struct TerrainProperties
{
  double lowerXBound = 1.0;
  double upperXBound = 2.0;
  double lowerZBound = 1.0;
  double upperZBound = 2.0;

  int destWidth;
  int destHeight;

  TerrainProperties(int destWidth, int destHeight) : destWidth(destWidth), destHeight(destHeight) {}
};

enum class DrawMode : GLenum
{
  TRIANGLES = GL_TRIANGLES,
  LINES = GL_LINES,
};

class World
{
private:
  Buffer vbo;
  VertexArray vao;
  Voxel::Manager voxels;
  std::vector<Vertex> vertices;

public:
  TerrainProperties terrain{Voxel::Chunk::ChunkSize *Voxel::Manager::Chunks, Voxel::Chunk::ChunkSize *Voxel::Manager::Chunks};
  DrawMode drawMode = DrawMode::TRIANGLES;

public:
  World() : vbo(BufferTarget::ARRAY_BUFFER, VertexDraw::DYNAMIC)
  {
    vao.generate();
    vbo.generate();
  }

  void draw()
  {
    vao.bind();
    glDrawArrays(static_cast<GLenum>(drawMode), 0, vertices.size());
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

    noise::module::Perlin perlin;
    perlin.SetSeed(static_cast<int>(std::time(0)));

    utils::NoiseMap heightMap;
    utils::NoiseMapBuilderPlane heightMapBuilder;

    heightMapBuilder.SetSourceModule(perlin);
    heightMapBuilder.SetDestNoiseMap(heightMap);
    heightMapBuilder.SetDestSize(terrain.destWidth, terrain.destHeight);
    heightMapBuilder.SetBounds(terrain.lowerXBound, terrain.upperXBound, terrain.lowerZBound, terrain.upperZBound);
    heightMapBuilder.Build();

    for (int z = 0; z < Voxel::Chunk::ChunkSize * Voxel::Manager::Chunks; ++z)
    {
      for (int x = 0; x < Voxel::Chunk::ChunkSize * Voxel::Manager::Chunks; ++x)
      {
        float n = heightMap.GetValue(x, z);
        unsigned int height = static_cast<unsigned int>(std::round((std::clamp(n, -1.0f, 1.0f) + 1) * ((Voxel::Chunk::ChunkSize * Voxel::Manager::Chunks) / 2)));
        for (size_t y = 0; y < height; y++)
          voxels.set({x, y, z}, Voxel::Type::GRASS);
      }
    }

    voxels.greedyMesh(vertices);
    setBuffer();
  }

  void fill()
  {
    voxels.clear();

    const int size = Voxel::Chunk::ChunkSize * Voxel::Manager::Chunks;
    for (size_t z = 0; z < size; z++)
      for (size_t x = 0; x < size; x++)
        for (size_t y = 0; y < size; y++)
          voxels.set({x, y, z}, Voxel::Type::GRASS);

    voxels.greedyMesh(vertices);
    setBuffer();
  }

  void fillSphere()
  {
    voxels.clear();

    const unsigned int size = Voxel::Chunk::ChunkSize * Voxel::Manager::Chunks;
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

    voxels.greedyMesh(vertices);
    setBuffer();
  }
};