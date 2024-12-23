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

class World
{
private:
  VertexArray vao;
  Buffer vbo;
  Buffer ebo;

  Buffer ccc; // Center center chunk

  std::unordered_map<int, Voxel> voxels;
  UniformGrid3D<glm::ivec3(2, 5, 1)> grid;

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
    ccc.resize(0, grid.count() * sizeof(Voxel));

    vao.bind();

    fill();
    // fillSphere();
  }

  void setModel(Model *model)
  {
    ebo.set(model->getIndices());
    vbo.set(model->getVertices());

    vao.set(0, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, position));
    vao.set(1, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    vao.set(2, 2, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));
  };

  void generateVoxels()
  {
    float gap = 1.5f;

    grid.getColumn();

    for (size_t i = 0; i < grid.count(); i++)
    {
      glm::vec3 position = grid.getPosition(i);
      // std::cout << position.x << " " << position.y << " " << position.z << std::endl;

      // grid.getColumn(position);
      // https://www.youtube.com/watch?v=4xs66m1Of4A
      // std::cout << grid.getColumn(position) << std::endl;

      if (grid.getValue(i))
      {
        voxels[i].type = 1;
        voxels[i].setPosition(glm::vec3{position.x * gap, position.y * gap, position.z * gap});

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    }

    std::cout << "Voxels: " << voxels.size() << std::endl;
  }

  void generateVoxelsWithoutOptimization()
  {
    float gap = 1.5f;

    for (size_t i = 0; i < grid.count(); i++)
    {
      glm::vec3 position = grid.getPosition(i);

      if (grid.getValue(i))
      {
        voxels[i].type = 1;
        voxels[i].setPosition(glm::vec3{position.x * gap, position.y * gap, position.z * gap});

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
      }
    }

    std::cout << "Voxels: " << voxels.size() << std::endl;
  }

  void fill()
  {
    for (size_t i = 0; i < grid.count(); i++)
      grid.setValue(i, 1);
  }

  void fillSphere()
  {
    const glm::ivec3 center = grid.size() / 2;

    float radius = std::min({center.x, center.y, center.z}) - 1.0f;

    for (size_t i = 0; i < grid.count(); i++)
    {
      glm::ivec3 position = grid.getPosition(i);

      float dx = position.x - center.x;
      float dy = position.y - center.y;
      float dz = position.z - center.z;

      float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

      if (distance <= radius)
        grid.setValue(i, 1);
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
    return grid.count();
  }
};