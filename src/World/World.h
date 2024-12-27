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
#include <iostream>

// // Front face (2 triangles, CCW)
// vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
// vertices.push_back({position + glm::vec3(1.0f, column.size, 0.0f)});
// vertices.push_back({position + glm::vec3(1.0f, 0.0f, 0.0f)});

// vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
// vertices.push_back({position + glm::vec3(0.0f, column.size, 0.0f)});
// vertices.push_back({position + glm::vec3(1.0f, column.size, 0.0f)});

// // Back face (2 triangles, CCW)
// vertices.push_back({position + glm::vec3(0.0f, 0.0f, 1.0f)});
// vertices.push_back({position + glm::vec3(1.0f, 0.0f, 1.0f)});
// vertices.push_back({position + glm::vec3(1.0f, column.size, 1.0f)});

// vertices.push_back({position + glm::vec3(0.0f, 0.0f, 1.0f)});
// vertices.push_back({position + glm::vec3(1.0f, column.size, 1.0f)});
// vertices.push_back({position + glm::vec3(0.0f, column.size, 1.0f)});

// // Left face (2 triangles, CCW)
// vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
// vertices.push_back({position + glm::vec3(0.0f, 0.0f, 1.0f)});
// vertices.push_back({position + glm::vec3(0.0f, column.size, 1.0f)});

// vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
// vertices.push_back({position + glm::vec3(0.0f, column.size, 1.0f)});
// vertices.push_back({position + glm::vec3(0.0f, column.size, 0.0f)});

// // // Right face (2 triangles, CCW)
// vertices.push_back({position + glm::vec3(1.0f, 0.0f, 0.0f)});
// vertices.push_back({position + glm::vec3(1.0f, column.size, 1.0f)});
// vertices.push_back({position + glm::vec3(1.0f, 0.0f, 1.0f)});

// vertices.push_back({position + glm::vec3(1.0f, 0.0f, 0.0f)});
// vertices.push_back({position + glm::vec3(1.0f, column.size, 0.0f)});
// vertices.push_back({position + glm::vec3(1.0f, column.size, 1.0f)});

inline char createMask(unsigned int x)
{
  if (x == 64)
    return 0ULL;
  return ~((1ULL << x) - 1);
}

struct Info
{
  int size;
  int offset;
};

inline Info getInfo(uint64_t bits)
{
  int offset = __builtin_ffsll(bits);

  if (offset)
    bits = bits >> offset - 1;

  int size = __builtin_ctzll(~bits);

  return {
      size,
      --offset,
  };
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

    // grid.setValue(1, 0, 1, 1);
    // grid.setValue(1, 1, 1, 1);
    // grid.setValue(2, 0, 0, 0);
    // grid.setValue(3, 0, 0, 1);
    // grid.setValue(4, 0, 0, 1);
    // grid.setValue(5, 0, 0, 0);
    // grid.setValue(6, 0, 0, 1);

    // grid.setValue(0, 1, 0, 1);
    // grid.setValue(1, 1, 0, 1);
    // grid.setValue(0, 3, 0, 1);
    // grid.setValue(1, 3, 0, 1);

    // grid.setValue(0, 1, 1, 1);
    // grid.setValue(1, 1, 1, 1);

    // fill();
    fillSphere();
  }

  void generateMesh()
  {
    std::vector<Vertex> vertices;

    // Get all the cubes on each axis and create faces are relevant places
    // Do this for the x, y & z axis, thereby creating one mesh with minimal vertices and no extra faces

    UniformGrid3D voxels = grid;

    const glm::ivec3 &size = voxels.size();

    for (size_t z = 0; z < size.z; z++)
    {
      for (size_t x = 0; x < size.x; x++)
      {
        uint64_t &column = voxels.getColumn(x, 0, z);

        if (!column)
          continue;

        while (column)
        {
          Info iCol = getInfo(column);

          column &= createMask(iCol.size + iCol.offset);

          glm::vec3 position(x, iCol.offset, z);

          vertices.push_back({position + glm::vec3(0.0f, iCol.size, 0.0f)});
          vertices.push_back({position + glm::vec3(1.0f, iCol.size, 0.0f)});
          vertices.push_back({position + glm::vec3(1.0f, iCol.size, 1.0f)});

          vertices.push_back({position + glm::vec3(0.0f, iCol.size, 0.0f)});
          vertices.push_back({position + glm::vec3(1.0f, iCol.size, 1.0f)});
          vertices.push_back({position + glm::vec3(0.0f, iCol.size, 1.0f)});

          vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
          vertices.push_back({position + glm::vec3(1.0f, 0.0f, 1.0f)});
          vertices.push_back({position + glm::vec3(1.0f, 0.0f, 0.0f)});

          vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
          vertices.push_back({position + glm::vec3(0.0f, 0.0f, 1.0f)});
          vertices.push_back({position + glm::vec3(1.0f, 0.0f, 1.0f)});
        }
      }
    }

    // https://www.youtube.com/watch?v=4xs66m1Of4A

    vao.bind();
    vbo.set(vertices);
    vao.set(0, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, position));

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