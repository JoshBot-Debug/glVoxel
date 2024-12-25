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
// vertices.push_back({position + glm::vec3(1.0f, height, 0.0f)});
// vertices.push_back({position + glm::vec3(1.0f, 0.0f, 0.0f)});

// vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
// vertices.push_back({position + glm::vec3(0.0f, height, 0.0f)});
// vertices.push_back({position + glm::vec3(1.0f, height, 0.0f)});

// // Back face (2 triangles, CCW)
// vertices.push_back({position + glm::vec3(0.0f, 0.0f, 1.0f)});
// vertices.push_back({position + glm::vec3(1.0f, 0.0f, 1.0f)});
// vertices.push_back({position + glm::vec3(1.0f, height, 1.0f)});

// vertices.push_back({position + glm::vec3(0.0f, 0.0f, 1.0f)});
// vertices.push_back({position + glm::vec3(1.0f, height, 1.0f)});
// vertices.push_back({position + glm::vec3(0.0f, height, 1.0f)});

// // Left face (2 triangles, CCW)
// vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
// vertices.push_back({position + glm::vec3(0.0f, 0.0f, 1.0f)});
// vertices.push_back({position + glm::vec3(0.0f, height, 1.0f)});

// vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
// vertices.push_back({position + glm::vec3(0.0f, height, 1.0f)});
// vertices.push_back({position + glm::vec3(0.0f, height, 0.0f)});

// // // Right face (2 triangles, CCW)
// vertices.push_back({position + glm::vec3(1.0f, 0.0f, 0.0f)});
// vertices.push_back({position + glm::vec3(1.0f, height, 1.0f)});
// vertices.push_back({position + glm::vec3(1.0f, 0.0f, 1.0f)});

// vertices.push_back({position + glm::vec3(1.0f, 0.0f, 0.0f)});
// vertices.push_back({position + glm::vec3(1.0f, height, 0.0f)});
// vertices.push_back({position + glm::vec3(1.0f, height, 1.0f)});

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

    grid.setValue(0, 1, 0, 1);
    grid.setValue(1, 1, 0, 1);

    grid.setValue(0, 2, 0, 1);
    grid.setValue(1, 2, 0, 1);

    grid.setValue(0, 1, 1, 1);
    grid.setValue(1, 1, 1, 1);

    grid.setValue(0, 6, 0, 1);
    grid.setValue(1, 6, 0, 1);

    grid.setValue(0, 7, 0, 1);
    grid.setValue(1, 7, 0, 1);

    grid.setValue(0, 6, 1, 1);
    grid.setValue(1, 6, 1, 1);

    // fill();
    // fillSphere();
  }

  void generateMesh()
  {
    std::vector<Vertex> vertices;

    // Get all the cubes on each axis and create faces are relevant places
    // Do this for the x, y & z axis, thereby creating one mesh with minimal vertices and no extra faces

    const glm::ivec3 &size = grid.size();

    for (unsigned int z = 0; z < size.z; z++)
    {
      for (unsigned int x = 0; x < size.x; x++)
      {
        uint64_t column = grid.getColumn(x, 0, z);

        glm::vec3 position(x, -1, z);

        while (column != 0)
        {
          ++position.y;

          unsigned int height = __builtin_ctzll(~column);

          column = column >> (height + 1);

          if (!height)
            continue;

          vertices.push_back({position + glm::vec3(0.0f, height, 0.0f)});
          vertices.push_back({position + glm::vec3(1.0f, height, 0.0f)});
          vertices.push_back({position + glm::vec3(1.0f, height, 1.0f)});

          vertices.push_back({position + glm::vec3(0.0f, height, 0.0f)});
          vertices.push_back({position + glm::vec3(1.0f, height, 1.0f)});
          vertices.push_back({position + glm::vec3(0.0f, height, 1.0f)});

          vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
          vertices.push_back({position + glm::vec3(1.0f, 0.0f, 1.0f)});
          vertices.push_back({position + glm::vec3(1.0f, 0.0f, 0.0f)});

          vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
          vertices.push_back({position + glm::vec3(0.0f, 0.0f, 1.0f)});
          vertices.push_back({position + glm::vec3(1.0f, 0.0f, 1.0f)});

          position.y += height;
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

    float radius = std::min({center.x, center.y, center.z}) - 1.0f;

    for (size_t z = 0; z < size.z; z++)
      for (size_t x = 0; x < size.x; x++)
        for (size_t y = 0; y < size.y; y++)
        {
          float dx = x - center.x;
          float dy = y - center.y;
          float dz = z - center.z;

          float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

          if (distance <= radius)
            grid.setValue(x, y, z, 1);
        }
  }
};