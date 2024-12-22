#pragma once

#include <algorithm>

#include "Engine/Core/Buffer.h"
#include "Engine/Core/VertexArray.h"
#include "Engine/Model.h"
#include "World/Voxel.h"
#include "World/OctreeNode.h"

class World
{
private:
  VertexArray vao;
  Buffer vbo;
  Buffer ebo;

  Buffer ccc; // Center center chunk

  OctreeNode<Voxel> root;
  glm::ivec3 size;
  std::vector<unsigned int> grid;

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

    vao.bind();
  }

  void setModel(Model *model)
  {
    ebo.set(model->getIndices());
    vbo.set(model->getVertices());

    vao.set(0, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, position));
    vao.set(1, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    vao.set(2, 2, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));
  };

  void createSphere(glm::ivec3 size)
  {
    this->size = size;

    unsigned int totalSize = size.x * size.y * size.z;

    grid.resize(totalSize, 0);

    float centerX = static_cast<float>(size.x) / 2.0f;
    float centerY = static_cast<float>(size.y) / 2.0f;
    float centerZ = static_cast<float>(size.z) / 2.0f;

    float radius = std::min({centerX, centerY, centerZ}) - 1.0f;

    for (unsigned int x = 0; x < size.x; ++x)
    {
      for (unsigned int y = 0; y < size.y; ++y)
      {
        for (unsigned int z = 0; z < size.z; ++z)
        {
          float dx = x - centerX;
          float dy = y - centerY;
          float dz = z - centerZ;
          float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

          if (distance <= radius)
            grid[x + size.x * (y + size.y * z)] = 1;
        }
      }
    }
  }

  void createGrid(glm::ivec3 size)
  {
    this->size = size;

    grid.resize(size.x * size.y * size.z, 0);

    for (unsigned int x = 0; x < size.x; ++x)
      for (unsigned int y = 0; y < size.y; ++y)
        for (unsigned int z = 0; z < size.z; ++z)
          grid[x + size.x * (y + size.y * z)] = 1;
  }

  void generateInstances()
  {
    std::cout << "Generating voxels" << std::endl;

    ccc.addPartition(0);
    ccc.resize(0, grid.size() * sizeof(Voxel));

    std::cout << "Resized buffer: " << grid.size() * sizeof(Voxel) << std::endl;

    int cx = size.x / 2;
    int cy = size.y / 2;
    int cz = size.z / 2;

    float gap = 1.5f;

    for (int z = -cz; z < cz; ++z)
    {
      for (int y = -cy; y < cy; ++y)
      {
        for (int x = -cx; x < cx; ++x)
        {
          Voxel voxel;
          int index = (x + cx) + size.x * ((y + cy) + size.y * (z + cz));

          if (grid[index])
          {
            voxel.type = 1;
            voxel.setPosition(glm::vec3{x * gap, y * gap, z * gap});
            ccc.upsert(sizeof(Voxel), index, sizeof(Voxel), (const void *)&voxel);
          }
        }
      }
    }

    setVertexAttribPointer();
  }

  void addVoxel(const Voxel &voxel)
  {
    ccc.set(sizeof(Voxel), 1, (const void *)&voxel);
    setVertexAttribPointer();
  }

  void update(std::vector<Voxel> voxels)
  {
    ccc.update(0, voxels);
  }

  const unsigned int getInstancesCount()
  {
    return grid.size();
  }
};