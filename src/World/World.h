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

    grid.setValue({0, 0, 0}, 1);
    grid.setValue({0, 1, 0}, 1);
    grid.setValue({0, 2, 0}, 1);
    grid.setValue({0, 3, 0}, 1);
    grid.setValue({0, 4, 0}, 1);
    grid.setValue({0, 5, 0}, 1);
    // fill();
    // fillSphere();
  }

  void generateMesh()
  {
    std::vector<Vertex> vertices;

    const glm::ivec3 &size = grid.size();

    for (unsigned int z = 0; z < size.z; z++)
    {
      for (unsigned int x = 0; x < size.x; x++)
      {
        uint64_t &column = grid.getColumn(x, z);
        unsigned int height = __builtin_ctzll(~column);

        if (height == 0ULL)
          continue;

        std::cout << "Height: " << height << std::endl;

        // Calculate base position for the current block
        glm::vec3 position(x, 0.0f, z);

        // Front face (2 triangles, CCW)
        vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
        vertices.push_back({position + glm::vec3(1.0f, height, 0.0f)});
        vertices.push_back({position + glm::vec3(1.0f, 0.0f, 0.0f)});

        vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
        vertices.push_back({position + glm::vec3(0.0f, height, 0.0f)});
        vertices.push_back({position + glm::vec3(1.0f, height, 0.0f)});

        // Back face (2 triangles, CCW)
        vertices.push_back({position + glm::vec3(0.0f, 0.0f, 1.0f)});
        vertices.push_back({position + glm::vec3(1.0f, 0.0f, 1.0f)});
        vertices.push_back({position + glm::vec3(1.0f, height, 1.0f)});

        vertices.push_back({position + glm::vec3(0.0f, 0.0f, 1.0f)});
        vertices.push_back({position + glm::vec3(1.0f, height, 1.0f)});
        vertices.push_back({position + glm::vec3(0.0f, height, 1.0f)});

        // Left face (2 triangles, CCW)
        vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
        vertices.push_back({position + glm::vec3(0.0f, 0.0f, 1.0f)});
        vertices.push_back({position + glm::vec3(0.0f, height, 1.0f)});

        vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
        vertices.push_back({position + glm::vec3(0.0f, height, 1.0f)});
        vertices.push_back({position + glm::vec3(0.0f, height, 0.0f)});

        // // Right face (2 triangles, CCW)
        vertices.push_back({position + glm::vec3(1.0f, 0.0f, 0.0f)});
        vertices.push_back({position + glm::vec3(1.0f, height, 1.0f)});
        vertices.push_back({position + glm::vec3(1.0f, 0.0f, 1.0f)});

        vertices.push_back({position + glm::vec3(1.0f, 0.0f, 0.0f)});
        vertices.push_back({position + glm::vec3(1.0f, height, 0.0f)});
        vertices.push_back({position + glm::vec3(1.0f, height, 1.0f)});

        // // Top face (2 triangles, CCW)
        vertices.push_back({position + glm::vec3(0.0f, height, 0.0f)});
        vertices.push_back({position + glm::vec3(1.0f, height, 1.0f)});
        vertices.push_back({position + glm::vec3(1.0f, height, 0.0f)});

        vertices.push_back({position + glm::vec3(0.0f, height, 0.0f)});
        vertices.push_back({position + glm::vec3(0.0f, height, 1.0f)});
        vertices.push_back({position + glm::vec3(1.0f, height, 1.0f)});

        // Bottom face (2 triangles, CCW)
        vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
        vertices.push_back({position + glm::vec3(1.0f, 0.0f, 0.0f)});
        vertices.push_back({position + glm::vec3(1.0f, 0.0f, 1.0f)});

        vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
        vertices.push_back({position + glm::vec3(1.0f, 0.0f, 1.0f)});
        vertices.push_back({position + glm::vec3(0.0f, 0.0f, 1.0f)});
      }
    }

    // https://www.youtube.com/watch?v=4xs66m1Of4A

    vao.bind();
    vbo.set(vertices);
    // ebo.set(indices);
    vao.set(0, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, position));

    std::cout << "Vertices: " << vertices.size() << std::endl;
    // std::cout << "Indices: " << indices.size() << std::endl;
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
};