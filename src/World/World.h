#pragma once

#include <algorithm>

#include "Engine/Core/Buffer.h"
#include "Engine/Core/VertexArray.h"
#include "Engine/Model.h"
#include "World/Voxel.h"
#include "World/OctreeNode.h"
#include "World/UniformGrid3D.h"

#include <thread> // For std::this_thread
#include <chrono> // For std::chrono

template <size_t N>
unsigned long long getTrailingOnes(const std::bitset<N> &bs)
{
  // Convert the bitset to an unsigned long long for bit manipulation
  unsigned long long value = bs.to_ullong();

  // Isolate trailing 1's (using two's complement trick)
  unsigned long long trailingOnes = value & -value;

  // Convert back to bitset
  return trailingOnes;
}

template <size_t N>
unsigned long long getTrailingZeros(const std::bitset<N> &bs)
{
  // Convert the bitset to an unsigned long long for bit manipulation
  unsigned long long value = bs.to_ullong();

  // Isolate trailing 0's by taking complement and applying the two's complement trick
  unsigned long long trailingZeros = (~value) & -(~value);

  // Convert back to bitset
  return trailingZeros;
}

class World
{
private:
  VertexArray vao;
  Buffer vbo;
  Buffer ebo;

  Buffer ccc; // Center center chunk

  std::unordered_map<int, Voxel> voxels;
  UniformGrid3D<glm::ivec3(10)> grid;

private:
  void setVertexAttribPointer()
  {
    vao.bind();
    vao.set(3, 1, VertexType::UNSIGNED_INT, false, sizeof(Voxel), (void *)offsetof(Voxel, type), 1);
    vao.set(4, 4, VertexType::FLOAT, false, sizeof(Voxel), (void *)offsetof(Voxel, identity), 1);
    vao.set(5, 4, VertexType::FLOAT, false, sizeof(Voxel), (void *)(offsetof(Voxel, identity) + (sizeof(float) * 4)), 1);
    vao.set(6, 4, VertexType::FLOAT, false, sizeof(Voxel), (void *)(offsetof(Voxel, identity) + (sizeof(float) * 8)), 1);
    vao.set(7, 4, VertexType::FLOAT, false, sizeof(Voxel), (void *)(offsetof(Voxel, identity) + (sizeof(float) * 12)), 1);
  }

public:
  World() : vbo(BufferTarget::ARRAY_BUFFER),
            ebo(BufferTarget::ELEMENT_ARRAY_BUFFER),
            ccc(BufferTarget::ARRAY_BUFFER, VertexDraw::DYNAMIC)
  {
    vao.generate();
    vbo.generate();
    ebo.generate();

    ccc.generate();
    ccc.resize(0, grid.getCount() * sizeof(Voxel));

    vao.bind();

    // fillCube();
    fillSphere();
    // fillInvertedSphere();
  }

  void setModel(Model *model)
  {
    ebo.set(model->getIndices());
    vbo.set(model->getVertices());

    vao.set(0, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, position));
    vao.set(1, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    vao.set(2, 2, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));
  };

  void generateInstances3()
  {
    float gap = 1.0f;

    const auto &g = grid.getGrid();
    const auto &c = grid.getCenter();

    for (int x = -c.x; x < c.x; x++)
    {
      std::cout << "Next X" << std::endl;

      for (int y = -c.y; y < c.y; y++)
      {
        const auto &row = grid.getRow(x, y);

        if (!row.any())
          continue;

        std::cout << row << std::endl;

        for (int z = -c.z; z < c.z; z++)
        {
          if (grid.getValue(x, y, z))
          {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            const auto &index = grid.getIndex(x, y, z);
            voxels[index].type = 1;
            voxels[index].setPosition(glm::vec3{x * gap, y * gap, z * gap});
          }
        }
      }
    }

    // std::cout << "Voxels: " << voxels.size() << std::endl;
  }

  void generateInstances2()
  {
    ccc.bind();

    float gap = 1.5f;

    std::unordered_map<int, Voxel> voxels;

    const auto &g = grid.getGrid();
    const auto &c = grid.getCenter();

    for (int x = -c.x; x < c.x; x++)
    {
      for (int y = -c.y; y < c.y; y++)
      {
        for (int z = -c.z; z < c.z; z++)
        {
          if (grid.getValue(x, y, z))
          {
            const auto &index = grid.getIndex(x, y, z);
            voxels[index].type = 1;
            voxels[index].setPosition(glm::vec3{x * gap, y * gap, z * gap});
          }
        }
      }
    }

    for (const auto &voxel : voxels)
      ccc.upsert(sizeof(Voxel), voxel.first, sizeof(Voxel), (const void *)&voxel.second);

    // std::cout << "Voxels: " << voxels.size() << std::endl;

    setVertexAttribPointer();
  }

  void generateInstances()
  {
    ccc.bind();

    float gap = 1.5f;

    std::unordered_map<int, Voxel> voxels;

    for (size_t i = 0; i < grid.getCount(); i++)
    {
      glm::vec3 position = grid.getPosition(i);

      if (grid.getValue(i))
      {
        voxels[i].type = 1;
        voxels[i].setPosition(glm::vec3{position.x * gap, position.y * gap, position.z * gap});
      }
    }

    for (const auto &voxel : voxels)
      ccc.upsert(sizeof(Voxel), voxel.first, sizeof(Voxel), (const void *)&voxel.second);

    // std::cout << "Voxels: " << voxels.size() << std::endl;

    setVertexAttribPointer();
  }

  void fillCube()
  {
    const glm::ivec3 size = grid.getSize();

    for (unsigned int x = 0; x < size.x; ++x)
      for (unsigned int y = 0; y < size.y; ++y)
        for (unsigned int z = 0; z < size.z; ++z)
          grid.setValue(x + size.x * (y + size.y * z), 1);
  }

  void fillSphere()
  {
    const glm::ivec3 size = grid.getSize();
    float cx = static_cast<float>(size.x) / 2.0f;
    float cy = static_cast<float>(size.y) / 2.0f;
    float cz = static_cast<float>(size.z) / 2.0f;

    float radius = std::min({cx, cy, cz}) - 1.0f;

    for (unsigned int x = 0; x < size.x; ++x)
    {
      for (unsigned int y = 0; y < size.y; ++y)
      {
        for (unsigned int z = 0; z < size.z; ++z)
        {
          float dx = x - cx;
          float dy = y - cy;
          float dz = z - cz;
          float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

          if (distance <= radius)
            grid.setValue(x + size.x * (y + size.y * z), 1);
        }
      }
    }
  }

  void fillInvertedSphere()
  {
    const glm::ivec3 size = grid.getSize();
    float cx = static_cast<float>(size.x) / 2.0f;
    float cy = static_cast<float>(size.y) / 2.0f;
    float cz = static_cast<float>(size.z) / 2.0f;

    float radius = std::min({cx, cy, cz}) - 1.0f;

    for (unsigned int x = 0; x < size.x; ++x)
    {
      for (unsigned int y = 0; y < size.y; ++y)
      {
        for (unsigned int z = 0; z < size.z; ++z)
        {
          float dx = x - cx;
          float dy = y - cy;
          float dz = z - cz;
          float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

          if (distance >= radius)
            grid.setValue(x + size.x * (y + size.y * z), 1);
        }
      }
    }
  }

  void update()
  {
    ccc.bind();
    for (const auto &voxel : voxels)
      ccc.upsert(sizeof(Voxel), voxel.first, sizeof(Voxel), (const void *)&voxel.second);
    setVertexAttribPointer();
  }

  const unsigned int getInstancesCount()
  {
    return grid.getCount();
  }
};