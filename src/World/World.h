#pragma once

#include <algorithm>

#include "Engine/Core/Buffer.h"
#include "Engine/Core/VertexArray.h"
#include "Engine/Model.h"
#include "World/Voxel.h"
#include "World/OctreeNode.h"
#include "World/UniformGrid3D.h"

#include <iostream>

inline uint32_t createMask(unsigned int x)
{
  if (x == 32)
    return 0U;
  return ~((1U << x) - 1);
}

struct Info
{
  int size;
  int offset;
};

inline Info getInfo(uint32_t bits)
{
  int offset = __builtin_ffs(bits) - 1;

  if (offset > 0)
    bits = bits >> offset;

  int size = __builtin_ctz(~bits);

  return {
      size,
      offset,
  };
}

enum class FaceDirection
{
  TOP,
  BOTTOM,
  FRONT,
  BACK,
  LEFT,
  RIGHT
};

inline void generateFace(std::vector<Vertex> &vertices, glm::vec3 position, glm::vec3 size, FaceDirection direction)
{
  switch (direction)
  {
  case FaceDirection::TOP:
    vertices.emplace_back(Vertex{position + glm::vec3(0.0f, size.y, 0.0f), {0.0f, 1.0f, 0.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(size.x, size.y, size.z), {0.0f, 1.0f, 0.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(size.x, size.y, 0.0f), {0.0f, 1.0f, 0.0f}});

    vertices.emplace_back(Vertex{position + glm::vec3(0.0f, size.y, 0.0f), {0.0f, 1.0f, 0.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(0.0f, size.y, size.z), {0.0f, 1.0f, 0.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(size.x, size.y, size.z), {0.0f, 1.0f, 0.0f}});
    break;
  case FaceDirection::BOTTOM:
    vertices.emplace_back(Vertex{position + glm::vec3(0.0f, 0.0f, 0.0f), {0.0f, -1.0f, 0.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(size.x, 0.0f, 0.0f), {0.0f, -1.0f, 0.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(size.x, 0.0f, size.z), {0.0f, -1.0f, 0.0f}});

    vertices.emplace_back(Vertex{position + glm::vec3(0.0f, 0.0f, 0.0f), {0.0f, -1.0f, 0.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(size.x, 0.0f, size.z), {0.0f, -1.0f, 0.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(0.0f, 0.0f, size.z), {0.0f, -1.0f, 0.0f}});
    break;
  case FaceDirection::FRONT:
    vertices.emplace_back(Vertex{position + glm::vec3(0.0f, 0.0f, 0.0f), {0.0f, 0.0f, -1.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(size.x, size.y, 0.0f), {0.0f, 0.0f, -1.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(size.x, 0.0f, 0.0f), {0.0f, 0.0f, -1.0f}});

    vertices.emplace_back(Vertex{position + glm::vec3(0.0f, 0.0f, 0.0f), {0.0f, 0.0f, -1.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(0.0f, size.y, 0.0f), {0.0f, 0.0f, -1.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(size.x, size.y, 0.0f), {0.0f, 0.0f, -1.0f}});
    break;
  case FaceDirection::BACK:
    vertices.emplace_back(Vertex{position + glm::vec3(0.0f, 0.0f, size.z), {0.0f, 0.0f, 1.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(size.x, 0.0f, size.z), {0.0f, 0.0f, 1.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(size.x, size.y, size.z), {0.0f, 0.0f, 1.0f}});

    vertices.emplace_back(Vertex{position + glm::vec3(0.0f, 0.0f, size.z), {0.0f, 0.0f, 1.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(size.x, size.y, size.z), {0.0f, 0.0f, 1.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(0.0f, size.y, size.z), {0.0f, 0.0f, 1.0f}});
    break;
  case FaceDirection::LEFT:
    vertices.emplace_back(Vertex{position + glm::vec3(0.0f, 0.0f, 0.0f), {-1.0f, 0.0f, 0.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(0.0f, 0.0f, size.z), {-1.0f, 0.0f, 0.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(0.0f, size.y, size.z), {-1.0f, 0.0f, 0.0f}});

    vertices.emplace_back(Vertex{position + glm::vec3(0.0f, 0.0f, 0.0f), {-1.0f, 0.0f, 0.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(0.0f, size.y, size.z), {-1.0f, 0.0f, 0.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(0.0f, size.y, 0.0f), {-1.0f, 0.0f, 0.0f}});
    break;
  case FaceDirection::RIGHT:
    vertices.emplace_back(Vertex{position + glm::vec3(size.x, 0.0f, 0.0f), {1.0f, 0.0f, 0.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(size.x, size.y, size.z), {1.0f, 0.0f, 0.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(size.x, 0.0f, size.z), {1.0f, 0.0f, 0.0f}});

    vertices.emplace_back(Vertex{position + glm::vec3(size.x, 0.0f, 0.0f), {1.0f, 0.0f, 0.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(size.x, size.y, 0.0f), {1.0f, 0.0f, 0.0f}});
    vertices.emplace_back(Vertex{position + glm::vec3(size.x, size.y, size.z), {1.0f, 0.0f, 0.0f}});
    break;
  default:
    break;
  }
}

class World
{
private:
  VertexArray vao;
  Buffer vbo;
  Buffer ebo;

  UniformGrid3D grid;

public:
  World() : vbo(BufferTarget::ARRAY_BUFFER),
            ebo(BufferTarget::ELEMENT_ARRAY_BUFFER)
  {
    vao.generate();
    vbo.generate();
    ebo.generate();

    grid.setValue(0, 0, 0, 1);

    // fill();
    fillSphere();
  }

  void generateMesh(std::vector<Vertex> &vertices)
  {
    UniformGrid3D voxels = grid;
    const glm::ivec3 &size = voxels.size();

    for (size_t z = 0; z < size.z; z++)
    {
      for (size_t x = 0; x < size.x; x++)
      {
        uint32_t &column = voxels.getColumn(x, 0, z);

        while (column)
        {
          Info iCol = getInfo(column);
          column &= createMask(iCol.size + iCol.offset);

          glm::vec3 position(x, iCol.offset, z);
          glm::vec3 size(1.0f, iCol.size, 1.0f);

          generateFace(vertices, position, size, FaceDirection::TOP);
          generateFace(vertices, position, size, FaceDirection::BOTTOM);
        }
      }
    }

    for (size_t z = 0; z < size.z; z++)
    {
      for (size_t y = 0; y < size.y; y++)
      {
        uint32_t &row = voxels.getRow(0, y, z);

        while (row)
        {
          Info iRow = getInfo(row);
          row &= createMask(iRow.size + iRow.offset);

          glm::vec3 position(iRow.offset, y, z);
          glm::vec3 size(iRow.size, 1.0f, 1.0f);

          generateFace(vertices, position, size, FaceDirection::LEFT);
          generateFace(vertices, position, size, FaceDirection::RIGHT);
        }
      }
    }

    for (size_t x = 0; x < size.x; x++)
    {
      for (size_t y = 0; y < size.y; y++)
      {
        uint32_t &depth = voxels.getDepth(x, y, 0);

        while (depth)
        {
          Info iDepth = getInfo(depth);
          depth &= createMask(iDepth.size + iDepth.offset);

          glm::vec3 position(x, y, iDepth.offset);
          glm::vec3 size(1.0f, 1.0f, iDepth.size);

          generateFace(vertices, position, size, FaceDirection::FRONT);
          generateFace(vertices, position, size, FaceDirection::BACK);
        }
      }
    }

    // https://www.youtube.com/watch?v=4xs66m1Of4A
  }

  void setBuffer(std::vector<Vertex> &vertices)
  {
    vao.bind();
    vbo.set(vertices);
    vao.set(0, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)(offsetof(Vertex, position)));
    vao.set(1, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)(offsetof(Vertex, normal)));

    std::cout << "Vertices: " << vertices.size() << std::endl;
  }

  void fill()
  {
    const glm::ivec3 size = grid.size();

    for (size_t z = 0; z < size.z; z++)
      for (size_t x = 0; x < size.x; x++)
        for (size_t y = 0; y < size.y; y++)
          grid.setValue(x, y, z, 1);
  }

  void fillSphere()
  {
    const glm::ivec3 size = grid.size();
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
            grid.setValue(x, y, z, 1);
        }
  }
};