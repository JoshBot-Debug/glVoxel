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

inline uint64_t createMask(unsigned int x)
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

inline Info getInfo(uint64_t &data)
{
  uint64_t _data = data;

  int offset = __builtin_ffsll(_data);

  if (offset)
    _data = _data >> offset - 1;

  int size = __builtin_ctzll(~_data);

  return {
      size,
      offset,
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

    grid.setValue(0, 0, 0, 1);
    grid.setValue(1, 0, 0, 1);

    // fill();
    // fillSphere();
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
      for (unsigned int x = 0; x < size.x; x++)
      {
        uint64_t &c = voxels.getColumn(x, 0, z);

        while (c)
        {
          Info column = getInfo(c);

          uint64_t &tr = voxels.getRow(0, column.size - 1, z);
          uint64_t &td = voxels.getDepth(x, column.size - 1, 0);
          Info row = getInfo(tr);
          Info depth = getInfo(td);

          uint64_t rMask = createMask(row.size + row.offset - 1);
          uint64_t dMask = createMask(depth.size + depth.offset - 1);
          uint64_t cMask = createMask(column.size + column.offset - 1);

          std::cout << "x: " << x << " z:" << z << std::endl;
          std::cout << "Column: " << std::bitset<64>(c) << std::endl;
          std::cout << "Row: " << std::bitset<64>(tr) << std::endl;
          std::cout << "Depth: " << std::bitset<64>(td) << std::endl;
          
          std::cout << "Column Mask: " << std::bitset<64>(cMask) << std::endl;
          std::cout << "Row Mask: " << std::bitset<64>(rMask) << std::endl;
          std::cout << "Depth Mask: " << std::bitset<64>(dMask) << std::endl;
          std::cout << "H: " << column.size << " Wx: " << row.size << " Wz: " << depth.size << std::endl;

          c &= createMask(column.size + column.offset - 1);
          tr &= createMask(row.size + row.offset - 1);
          td &= createMask(depth.size + depth.offset - 1);

          std::cout << "Column After Mask: " << std::bitset<64>(c) << std::endl;
          std::cout << "Row After Mask: " << std::bitset<64>(tr) << std::endl;
          std::cout << "Depth After Mask: " << std::bitset<64>(td) << std::endl;

          // while (tr || td)
          // {
          //   if (!row.size || !depth.size)
          //     break;

          //   glm::vec3 position(x, -1, z);

          //   vertices.push_back({position + glm::vec3(0.0f, column.size, 0.0f)});
          //   vertices.push_back({position + glm::vec3(row.size - x, column.size, 0.0f)});
          //   vertices.push_back({position + glm::vec3(row.size - x, column.size, depth.size)});

          //   vertices.push_back({position + glm::vec3(0.0f, column.size, 0.0f)});
          //   vertices.push_back({position + glm::vec3(row.size - x, column.size, depth.size)});
          //   vertices.push_back({position + glm::vec3(0.0f, column.size, depth.size)});

          //   vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
          //   vertices.push_back({position + glm::vec3(row.size - x, 0.0f, depth.size)});
          //   vertices.push_back({position + glm::vec3(row.size - x, 0.0f, 0.0f)});

          //   vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
          //   vertices.push_back({position + glm::vec3(0.0f, 0.0f, depth.size)});
          //   vertices.push_back({position + glm::vec3(row.size - x, 0.0f, depth.size)});
          // }
        }
      }
    }

    // https://www.youtube.com/watch?v=4xs66m1Of4A

    vao.bind();
    vbo.set(vertices);
    vao.set(0, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, position));

    std::cout << "Vertices: " << vertices.size() << std::endl;
  }

  // void generateMesh()
  // {
  //   std::vector<Vertex> vertices;

  //   // Get all the cubes on each axis and create faces are relevant places
  //   // Do this for the x, y & z axis, thereby creating one mesh with minimal vertices and no extra faces

  //   const glm::ivec3 &size = grid.size();

  //   for (unsigned int z = 0; z < size.z; z++)
  //   {
  //     for (unsigned int x = 0; x < size.x; x++)
  //     {
  //       uint64_t column = grid.getColumn(x, 0, z);

  //       glm::vec3 position(x, -1, z);

  //       while (column != 0)
  //       {
  //         ++position.y;

  //         unsigned int height = __builtin_ctzll(~column);

  //         column = column >> (height + 1);

  //         if (!height)
  //           continue;

  //         // Create column
  //         vertices.push_back({position + glm::vec3(0.0f, column.size, 0.0f)});
  //         vertices.push_back({position + glm::vec3(1.0f, column.size, 0.0f)});
  //         vertices.push_back({position + glm::vec3(1.0f, column.size, 1.0f)});

  //         vertices.push_back({position + glm::vec3(0.0f, column.size, 0.0f)});
  //         vertices.push_back({position + glm::vec3(1.0f, column.size, 1.0f)});
  //         vertices.push_back({position + glm::vec3(0.0f, column.size, 1.0f)});

  //         vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
  //         vertices.push_back({position + glm::vec3(1.0f, 0.0f, 1.0f)});
  //         vertices.push_back({position + glm::vec3(1.0f, 0.0f, 0.0f)});

  //         vertices.push_back({position + glm::vec3(0.0f, 0.0f, 0.0f)});
  //         vertices.push_back({position + glm::vec3(0.0f, 0.0f, 1.0f)});
  //         vertices.push_back({position + glm::vec3(1.0f, 0.0f, 1.0f)});

  //         position.y += height;
  //       }
  //     }
  //   }

  //   // https://www.youtube.com/watch?v=4xs66m1Of4A

  //   vao.bind();
  //   vbo.set(vertices);
  //   vao.set(0, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, position));

  //   std::cout << "Vertices: " << vertices.size() << std::endl;
  // }

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