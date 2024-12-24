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

    // grid.setValue({0, 0, 0}, 1);
    // grid.setValue({0, 1, 0}, 1);
    // grid.setValue({0, 2, 0}, 1);
    // grid.setValue({0, 3, 0}, 1);
    // grid.setValue({0, 4, 0}, 1);
    // grid.setValue({0, 5, 0}, 1);
    fill();
    // fillSphere();
  }

  void generateMesh()
  {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    const glm::ivec3 &size = grid.size();

    for (unsigned int z = 0; z < size.z; z++)
    {
      for (unsigned int x = 0; x < size.x; x++)
      {
        uint64_t &column = grid.getColumn(x, z);
        unsigned int height = __builtin_ctzll(~column);

        if (!height)
          continue;

        std::cout << "Height: " << height << std::endl;

        // Calculate base position for the current block
        glm::vec3 position(x, 0.0f, z);

        // Add the vertices for the current block (each block has 8 vertices)
        vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});   // 0
        vertices.push_back({position + glm::vec3(1.0f, 0.0f, 0.0f)});   // 1
        vertices.push_back({position + glm::vec3(1.0f, height, 0.0f)}); // 2
        vertices.push_back({position + glm::vec3(0.0f, height, 0.0f)}); // 3
        vertices.push_back({position + glm::vec3(0.0f, 0.0f, 1.0f)});   // 4
        vertices.push_back({position + glm::vec3(1.0f, 0.0f, 1.0f)});   // 5
        vertices.push_back({position + glm::vec3(1.0f, height, 1.0f)}); // 6
        vertices.push_back({position + glm::vec3(0.0f, height, 1.0f)}); // 7

        // Indices for the 6 faces of the cube
        unsigned int baseIndex = vertices.size() - 8;

        // Front face (0, 1, 2, 2, 3, 0)
        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 0);

        // Back face (4, 5, 6, 6, 7, 4)
        indices.push_back(baseIndex + 4);
        indices.push_back(baseIndex + 5);
        indices.push_back(baseIndex + 6);
        indices.push_back(baseIndex + 6);
        indices.push_back(baseIndex + 7);
        indices.push_back(baseIndex + 4);

        // Left face (0, 3, 7, 7, 4, 0)
        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 7);
        indices.push_back(baseIndex + 7);
        indices.push_back(baseIndex + 4);
        indices.push_back(baseIndex + 0);

        // Right face (1, 5, 6, 6, 2, 1)
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 5);
        indices.push_back(baseIndex + 6);
        indices.push_back(baseIndex + 6);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 1);

        // Top face (3, 2, 6, 6, 7, 3)
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 6);
        indices.push_back(baseIndex + 6);
        indices.push_back(baseIndex + 7);
        indices.push_back(baseIndex + 3);

        // Bottom face (0, 1, 5, 5, 4, 0)
        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 5);
        indices.push_back(baseIndex + 5);
        indices.push_back(baseIndex + 4);
        indices.push_back(baseIndex + 0);
      }
    }

    // https://www.youtube.com/watch?v=4xs66m1Of4A

    vao.bind();
    vbo.set(vertices);
    ebo.set(indices);
    vao.set(0, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, position));

    std::cout << "Vertices: " << vertices.size() << std::endl;
    std::cout << "Indices: " << indices.size() << std::endl;
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