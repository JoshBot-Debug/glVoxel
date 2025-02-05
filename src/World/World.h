#pragma once

#include <algorithm>

#include "Engine/Core/Buffer.h"
#include "Engine/Core/VertexArray.h"
#include "Engine/Model.h"
#include "World/Chunk.h"
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
  Buffer vbo;
  VertexArray vao;
  ChunkManager chunks;
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
    chunks.clear();

    noise::module::Perlin perlin;
    perlin.SetSeed(static_cast<int>(std::time(0)));

    utils::NoiseMap heightMap;
    utils::NoiseMapBuilderPlane heightMapBuilder;

    heightMapBuilder.SetSourceModule(perlin);
    heightMapBuilder.SetDestNoiseMap(heightMap);
    heightMapBuilder.SetDestSize(Chunk::SIZE * ChunkManager::CHUNKS, Chunk::SIZE * ChunkManager::CHUNKS);
    heightMapBuilder.SetBounds(1.0, 2.0, 1.0, 2.0);
    heightMapBuilder.Build();

    for (int z = 0; z < Chunk::SIZE * ChunkManager::CHUNKS; ++z)
    {
      for (int x = 0; x < Chunk::SIZE * ChunkManager::CHUNKS; ++x)
      {
        float n = heightMap.GetValue(x, z);
        unsigned int height = static_cast<unsigned int>(std::round((std::clamp(n, -1.0f, 1.0f) + 1) * ((Chunk::SIZE * ChunkManager::CHUNKS) / 2)));
        for (size_t y = 0; y < height; y++)
          chunks.set(x, y, z, 1);
      }
    }

    chunks.update(vertices);
    setBuffer();
  }
};