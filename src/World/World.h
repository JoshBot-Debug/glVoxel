#pragma once

#include <algorithm>

#include "Engine/Core/Buffer.h"
#include "Engine/Core/VertexArray.h"
#include "Engine/Model.h"
#include "World/Voxel.h"
#include "World/Octree.h"
#include "World/UniformGrid3D.h"

#include <iostream>
#include <noise/noise.h>
#include <noise/noiseutils.h>
#include <ctime>
#include <bitset>

struct Info
{
  float size;
  unsigned int offset;
};

inline Info getInfo(uint32_t bits)
{
  unsigned int offset = __builtin_ffs(bits) - 1;

  if (offset > 0)
    bits = bits >> offset;

  float size = __builtin_ctz(~bits);

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

inline void generateVertices(std::vector<Vertex> &vertices, glm::vec3 position, glm::vec3 size, const FaceDirection &direction)
{
  switch (direction)
  {
  case FaceDirection::TOP:
    vertices.emplace_back(Vertex{position.x, position.y + size.y, position.z, 0});
    vertices.emplace_back(Vertex{position.x + size.x, position.y + size.y, position.z + size.z, 0});
    vertices.emplace_back(Vertex{position.x + size.x, position.y + size.y, position.z, 0});

    vertices.emplace_back(Vertex{position.x, position.y + size.y, position.z, 0});
    vertices.emplace_back(Vertex{position.x, position.y + size.y, position.z + size.z, 0});
    vertices.emplace_back(Vertex{position.x + size.x, position.y + size.y, position.z + size.z, 0});
    break;
  case FaceDirection::BOTTOM:
    vertices.emplace_back(Vertex{position.x, position.y, position.z, 1});
    vertices.emplace_back(Vertex{position.x + size.x, position.y, position.z, 1});
    vertices.emplace_back(Vertex{position.x + size.x, position.y, position.z + size.z, 1});

    vertices.emplace_back(Vertex{position.x, position.y, position.z, 1});
    vertices.emplace_back(Vertex{position.x + size.x, position.y, position.z + size.z, 1});
    vertices.emplace_back(Vertex{position.x, position.y, position.z + size.z, 1});
    break;
  case FaceDirection::FRONT:
    vertices.emplace_back(Vertex{position.x, position.y, position.z, 2});
    vertices.emplace_back(Vertex{position.x + size.x, position.y + size.y, position.z, 2});
    vertices.emplace_back(Vertex{position.x + size.x, position.y, position.z, 2});

    vertices.emplace_back(Vertex{position.x, position.y, position.z, 2});
    vertices.emplace_back(Vertex{position.x, position.y + size.y, position.z, 2});
    vertices.emplace_back(Vertex{position.x + size.x, position.y + size.y, position.z, 2});
    break;
  case FaceDirection::BACK:
    vertices.emplace_back(Vertex{position.x, position.y, position.z + size.z, 3});
    vertices.emplace_back(Vertex{position.x + size.x, position.y, position.z + size.z, 3});
    vertices.emplace_back(Vertex{position.x + size.x, position.y + size.y, position.z + size.z, 3});

    vertices.emplace_back(Vertex{position.x, position.y, position.z + size.z, 3});
    vertices.emplace_back(Vertex{position.x + size.x, position.y + size.y, position.z + size.z, 3});
    vertices.emplace_back(Vertex{position.x, position.y + size.y, position.z + size.z, 3});
    break;
  case FaceDirection::LEFT:
    vertices.emplace_back(Vertex{position.x, position.y, position.z, 4});
    vertices.emplace_back(Vertex{position.x, position.y, position.z + size.z, 4});
    vertices.emplace_back(Vertex{position.x, position.y + size.y, position.z + size.z, 4});

    vertices.emplace_back(Vertex{position.x, position.y, position.z, 4});
    vertices.emplace_back(Vertex{position.x, position.y + size.y, position.z + size.z, 4});
    vertices.emplace_back(Vertex{position.x, position.y + size.y, position.z, 4});
    break;
  case FaceDirection::RIGHT:
    vertices.emplace_back(Vertex{position.x + size.x, position.y, position.z, 5});
    vertices.emplace_back(Vertex{position.x + size.x, position.y + size.y, position.z + size.z, 5});
    vertices.emplace_back(Vertex{position.x + size.x, position.y, position.z + size.z, 5});

    vertices.emplace_back(Vertex{position.x + size.x, position.y, position.z, 5});
    vertices.emplace_back(Vertex{position.x + size.x, position.y + size.y, position.z, 5});
    vertices.emplace_back(Vertex{position.x + size.x, position.y + size.y, position.z + size.z, 5});
    break;
  default:
    break;
  }
}

struct Face
{
  float x;
  float y;
  float z;
  float h;
  float w;
};

inline void mergeXAxis(std::vector<Face> &faces)
{
  // std::sort(faces.begin(), faces.end(), [](const Face &a, const Face &b)
  //         {
  // if (a.z != b.z) return a.z < b.z;
  // if (a.y != b.y) return a.y < b.y;
  // return a.x < b.x; });

  std::vector<Face> merged;

  for (size_t i = 0; i < faces.size(); ++i)
  {
    Face current = faces[i];
    while (i + 1 < faces.size())
    {
      const Face &next = faces[i + 1];
      if (current.z == next.z && current.y == next.y &&
          current.x + current.w == next.x && current.h == next.h)
      {
        current.w += next.w;
        ++i;
      }
      else
        break;
    }

    merged.push_back(current);
  }

  faces = std::move(merged);
}

inline void mergeZAxis(std::vector<Face> &faces)
{
  std::sort(faces.begin(), faces.end(), [](const Face &a, const Face &b)
            {
    if (a.x != b.x) return a.x < b.x;
    if (a.y != b.y) return a.y < b.y;
    return a.z < b.z; });

  std::vector<Face> merged;

  for (size_t i = 0; i < faces.size(); ++i)
  {
    Face current = faces[i];
    while (i + 1 < faces.size())
    {
      const Face &next = faces[i + 1];
      if (current.x == next.x && current.y == next.y &&
          current.z + current.h == next.z && current.w == next.w)
      {
        current.h += next.h;
        ++i;
      }
      else
        break;
    }

    merged.push_back(current);
  }

  faces = std::move(merged);
}

class World
{
private:
  VertexArray vao;
  Buffer vbo;

  UniformGrid3D grid;
  std::vector<Vertex> vertices;
  Octree<UniformGrid3D> tree;

public:
  World() : vbo(BufferTarget::ARRAY_BUFFER, VertexDraw::DYNAMIC)
  {
    vao.generate();
    vbo.generate();
    // grid.setValue(0, 0, 0, 1);
    // generateNoise();
    fillSphere(grid.size() / 4);
    // fill(grid.size() / 8);
    // fill(grid.size());

    // for (size_t i = 0; i < 32; i++)
    //   grid.setValue(0, i, 0, 0);

    update();
    setBuffer();
  }

  void draw()
  {
    vao.bind();
    // glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glDrawArrays(GL_LINES, 0, vertices.size());
  }

  void update()
  {
    vertices.clear();
    UniformGrid3D voxels = grid;
    UniformGrid3D tmp = grid;
    UniformGrid3D hwd = grid;
    const glm::ivec3 &size = voxels.size();

    uint32_t mask[UniformGrid3D::GRID_SIZE] = {};

    for (unsigned int z = 0; z < 8; z++)
    {
      for (unsigned int x = 0; x < size.x; x++)
      {
        uint32_t &column = hwd.getColumn(x, 0, z);
        column &= ~(column << 1) | ~(column >> 1);
        Info iCol = getInfo(column);

        if (iCol.offset < 0)
          continue;

        unsigned int y = iCol.offset + iCol.size - 1;
        unsigned int index = x + (size.y * (y + (size.z * z)));
        // unsigned int index = z + (size.y * (y + (size.x * x)));
        // unsigned int index = y + (size.x * (x + (size.z * z)));

        // std::cout << std::bitset<32>(column) << std::endl;
        if (column & 1 << y)
        {
          mask[index / UniformGrid3D::BITS] |= (1U << (index % UniformGrid3D::BITS));
        }
        // std::cout << y << " " << (column & 1 << y) << std::endl;
      }
    }

    for (unsigned int z = 0; z < size.z; z++)
    {
      for (unsigned int x = 0; x < size.x; x++)
      {
        uint32_t &column = tmp.getColumn(x, 0, z);

        while (column)
        {
          Info iCol = getInfo(column);
          // if (iCol.size < 1)
          //   continue;

          unsigned int y = iCol.offset + iCol.size - 1;
          // unsigned int index = x + (size.y * (y + (size.z * z)));
          // unsigned int index = z + (size.y * (y + (size.x * x)));
          // unsigned int index = y + (size.x * (x + (size.z * z)));
          unsigned int index = (size.x * (x + (size.z * z)));

          uint32_t &row = tmp.getRow(0, y, z);
          uint32_t &depth = tmp.getDepth(x, y, 0);
          std::cout << x << " " << y << " " << z << " b " << std::bitset<32>(column) << std::endl;
          std::cout << x << " " << y << " " << z << " m " << std::bitset<32>(mask[index / UniformGrid3D::BITS]) << " " << std::bitset<32>(row) << " " << std::bitset<32>(depth) << std::endl;
          // generateVertices(vertices, position, size, FaceDirection::TOP);
          // generateVertices(vertices, position, size, FaceDirection::BOTTOM);
          column &= tmp.createMask(iCol.size + iCol.offset);
        }

        // Info info = getInfo(mask[index / UniformGrid3D::BITS]);

        // glm::vec3 position(x, info.offset, z);
        // glm::vec3 size(1.0f, info.size, 1.0f);

        // generateVertices(vertices, position, size, FaceDirection::TOP);
        // generateVertices(vertices, position, size, FaceDirection::BOTTOM);
      }
    }

    // std::vector<Face> faces;

    // for (float z = 0; z < size.z; z++)
    // {
    //   for (float x = 0; x < size.x; x++)
    //   {
    //     uint32_t &column = voxels.getColumn(x, 0, z);

    //     while (column)
    //     {
    //       Info iCol = getInfo(column);
    //       column &= voxels.createMask(iCol.size + iCol.offset);

    //       glm::vec3 position(x, iCol.offset, z);
    //       glm::vec3 size(1.0f, iCol.size, 1.0f);

    //       faces.emplace_back(Face{x, iCol.offset + iCol.size, z, 1, 1});
    //       // generateVertices(vertices, position, size, FaceDirection::TOP);
    //       // generateVertices(vertices, position, size, FaceDirection::BOTTOM);
    //     }
    //   }
    // }

    // mergeXAxis(faces);
    // mergeZAxis(faces);

    // for (size_t i = 0; i < faces.size(); i++)
    // {
    //   // std::cout << faces[i].x << " " << faces[i].y << " " << faces[i].z << " : " << faces[i].w << " " << faces[i].h << std::endl;
    //   vertices.emplace_back(Vertex{faces[i].x, faces[i].y, faces[i].z, 0});
    //   vertices.emplace_back(Vertex{faces[i].x + faces[i].w, faces[i].y, faces[i].z + faces[i].h, 0});
    //   vertices.emplace_back(Vertex{faces[i].x + faces[i].w, faces[i].y, faces[i].z, 0});
    //   vertices.emplace_back(Vertex{faces[i].x, faces[i].y, faces[i].z, 0});
    //   vertices.emplace_back(Vertex{faces[i].x, faces[i].y, faces[i].z + faces[i].h, 0});
    //   vertices.emplace_back(Vertex{faces[i].x + faces[i].w, faces[i].y, faces[i].z + faces[i].h, 0});
    // }

    for (float z = 0; z < size.z; z++)
    {
      for (float x = 0; x < size.x; x++)
      {
        uint32_t &column = voxels.getColumn(x, 0, z);

        while (column)
        {
          Info iCol = getInfo(column);
          column &= voxels.createMask(iCol.size + iCol.offset);

          glm::vec3 position(x, iCol.offset, z);
          glm::vec3 size(1.0f, iCol.size, 1.0f);

          generateVertices(vertices, position, size, FaceDirection::TOP);
          // generateVertices(vertices, position, size, FaceDirection::BOTTOM);
        }
      }
    }

    // for (size_t z = 0; z < size.z; z++)
    // {
    //   for (size_t y = 0; y < size.y; y++)
    //   {
    //     uint32_t &row = voxels.getRow(0, y, z);

    //     while (row)
    //     {
    //       Info iRow = getInfo(row);
    //       row &= voxels.createMask(iRow.size + iRow.offset);

    //       glm::vec3 position(iRow.offset, y, z);
    //       glm::vec3 size(iRow.size, 1.0f, 1.0f);

    //       generateVertices(vertices, position, size, FaceDirection::LEFT);
    //       generateVertices(vertices, position, size, FaceDirection::RIGHT);
    //     }
    //   }
    // }

    // for (size_t x = 0; x < size.x; x++)
    // {
    //   for (size_t y = 0; y < size.y; y++)
    //   {
    //     uint32_t &depth = voxels.getDepth(x, y, 0);

    //     while (depth)
    //     {
    //       Info iDepth = getInfo(depth);
    //       depth &= voxels.createMask(iDepth.size + iDepth.offset);

    //       glm::vec3 position(x, y, iDepth.offset);
    //       glm::vec3 size(1.0f, 1.0f, iDepth.size);

    //       generateVertices(vertices, position, size, FaceDirection::FRONT);
    //       generateVertices(vertices, position, size, FaceDirection::BACK);
    //     }
    //   }
    // }
  }

  void setBuffer()
  {
    vao.bind();
    vbo.set(vertices);
    vao.set(0, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)(offsetof(Vertex, x)));
    vao.set(1, 1, VertexType::FLOAT, false, sizeof(Vertex), (void *)(offsetof(Vertex, normal)));
  }

  void generateNoise()
  {
    fillNoise(grid.size());
    update();
    setBuffer();
  }

  void fillNoise(const glm::ivec3 &size)
  {
    grid.clear();

    noise::module::Perlin perlin;
    perlin.SetSeed(static_cast<int>(std::time(0)));

    utils::NoiseMap heightMap;
    utils::NoiseMapBuilderPlane heightMapBuilder;

    heightMapBuilder.SetSourceModule(perlin);
    heightMapBuilder.SetDestNoiseMap(heightMap);
    heightMapBuilder.SetDestSize(32, 32);
    heightMapBuilder.SetBounds(1.0, 2.0, 1.0, 2.0);
    heightMapBuilder.Build();

    for (int z = 0; z < size.z; ++z)
    {
      for (int x = 0; x < size.x; ++x)
      {
        float n = heightMap.GetValue(x, z);
        unsigned int height = static_cast<int>(std::round((15 * (std::clamp(n, -1.0f, 1.0f) + 1)))) + 1;
        for (size_t y = 0; y < height; y++)
          grid.setValue(x, y, z, 1);
      }
    }
  }

  void fill(const glm::ivec3 &size)
  {
    for (size_t z = 0; z < size.z; z++)
      for (size_t x = 0; x < size.x; x++)
        for (size_t y = 0; y < size.y; y++)
          grid.setValue(x, y, z, 1);
  }

  void fillPlane(const glm::ivec3 &size)
  {
    for (size_t z = 0; z < size.z; z++)
      for (size_t x = 0; x < size.x; x++)
        grid.setValue(x, 0, z, 1);
  }

  void fillSphere(const glm::ivec3 &size)
  {
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