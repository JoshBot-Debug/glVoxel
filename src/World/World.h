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

#include <chrono>
#include <thread>

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

inline void generateFace(std::vector<Vertex> &vertices, glm::vec3 position, glm::vec3 size, const FaceDirection &direction)
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

inline void greedyMesh(std::vector<Vertex> &vertices, UniformGrid3D &grid, FaceDirection dir)
{
  glm::ivec3 size = grid.size();

  bool used[size.x][size.y] = {false}; // Track merged areas

  for (size_t y = 0; y < size.y; y++)
  {
    for (size_t x = 0; x < size.x; x++)
    {
      if (used[x][y] || !grid.get(x, y, 0))
        continue; // Skip empty or merged voxels

      size_t maxWidth = 1;
      size_t maxHeight = 1;

      // Expand width
      while (x + maxWidth < size.x && grid.get(x + maxWidth, y, 0) && !used[x + maxWidth][y])
      {
        maxWidth++;
      }

      // Expand height
      bool canExpand = true;
      while (y + maxHeight < size.y && canExpand)
      {
        for (size_t i = 0; i < maxWidth; i++)
        {
          if (!grid.get(x + i, y + maxHeight, 0) || used[x + i][y + maxHeight])
          {
            canExpand = false;
            break;
          }
        }
        if (canExpand)
          maxHeight++;
      }

      // Mark area as used
      for (size_t i = 0; i < maxWidth; i++)
      {
        for (size_t j = 0; j < maxHeight; j++)
        {
          used[x + i][y + j] = true;
        }
      }

      glm::vec3 position = glm::vec3(x, y, 0);               // Base position of the quad
      glm::vec3 size = glm::vec3(maxWidth, maxHeight, 1.0f); // Extend in X and Y

      generateFace(vertices, position, size, dir);
    }
  }
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
    // grid.set(0, 0, 0, 1);
    // grid.set(0, 1, 0, 1);
    // grid.set(0, 2, 0, 1);
    // grid.set(1, 0, 0, 1);
    // grid.set(1, 1, 0, 1);
    // grid.set(1, 2, 0, 1);

    // grid.set(3, 0, 0, 1);
    // grid.set(3, 1, 0, 1);
    // grid.set(4, 0, 0, 1);
    // grid.set(4, 1, 0, 1);
    // grid.set(5, 0, 0, 1);
    // grid.set(5, 1, 0, 1);
    // grid.set(6, 0, 0, 1);
    // grid.set(6, 1, 0, 1);

    // grid.set(0, 0, 1, 1);
    // grid.set(0, 1, 1, 1);
    // grid.set(0, 2, 1, 1);
    // grid.set(1, 0, 1, 1);
    // grid.set(1, 1, 1, 1);
    // grid.set(1, 2, 1, 1);

    grid.set(3, 0, 1, 1);
    grid.set(3, 1, 1, 1);
    grid.set(4, 0, 1, 1);
    grid.set(4, 1, 1, 1);
    grid.set(5, 0, 1, 1);
    grid.set(5, 1, 1, 1);
    grid.set(6, 0, 1, 1);
    grid.set(6, 1, 1, 1);

    grid.set(4, 0, 2, 1);
    grid.set(4, 1, 2, 1);
    grid.set(5, 0, 2, 1);
    grid.set(5, 1, 2, 1);

    // generateNoise();
    // fillSphere(grid.size());
    // fill(grid.size());

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

    uint8_t wMask[UniformGrid3D::GRID_SIZE] = {};
    uint8_t hMask[UniformGrid3D::GRID_SIZE] = {};

    for (size_t z = 0; z < UniformGrid3D::SIZE; z++)
    {
      for (size_t x = 0; x < UniformGrid3D::SIZE; x++)
      {
        uint8_t &column = grid.getColumn(x, 0, z);
        uint8_t first = column & ~(column << 1);
        uint8_t last = column & ~(column >> 1);

        while (first)
        {
          unsigned int w = __builtin_ffs(first) - 1;

          unsigned int wi = x + (UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * z)));
          wMask[wi / UniformGrid3D::BITS] |= (1U << (wi % UniformGrid3D::BITS));

          unsigned int hi = z + (UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * x)));
          hMask[hi / UniformGrid3D::BITS] |= (1U << (hi % UniformGrid3D::BITS));

          first &= ~((1UL << w + 1) - 1);
        }
      }
    }

    for (size_t z = 0; z < UniformGrid3D::SIZE; z++)
    {
      for (size_t x = 0; x < UniformGrid3D::SIZE; x++)
      {
        uint8_t &column = grid.getColumn(x, 0, z);

        uint8_t first = column & ~(column << 1);
        uint8_t last = column & ~(column >> 1);

        while (first)
        {
          unsigned int w = __builtin_ffs(first) - 1;

          uint8_t &width = wMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * z))) / UniformGrid3D::BITS];
          const Info wInfo = getInfo(width);

          uint8_t &height = hMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * x))) / UniformGrid3D::BITS];
          const Info hInfo = getInfo(height);

          std::cout << "wzho " << w << " " << z << " " << hInfo.offset << std::endl;
          std::cout << "wxwo " << w << " " << x << " " << wInfo.offset << std::endl;
          std::cout << "c " << std::bitset<8>(column) << std::endl;
          std::cout << "h " << std::bitset<8>(height) << std::endl;
          std::cout << "w " << std::bitset<8>(width) << std::endl;

          first &= ~((1UL << w + 1) - 1);

          if (hInfo.offset != z || wInfo.offset != x)
            continue;

          generateFace(vertices, {x, w, z}, {wInfo.size, 1.0f, hInfo.size}, FaceDirection::BOTTOM);

          width &= ~((1UL << (wInfo.offset + (int)wInfo.size)) - 1);
          height &= ~((1UL << (hInfo.offset + (int)hInfo.size)) - 1);
        }

        //     while (last)
        //     {
        //       unsigned int w = __builtin_ffs(last) - 1;
        //       generateFace(vertices, {x, w, z}, {1.0f, 1.0f, 1.0f}, FaceDirection::TOP);
        //       last &= ~((1UL << w + 1) - 1);
        //     }
      }
    }

    // for (size_t z = 0; z < UniformGrid3D::SIZE; z++)
    // {
    //   for (size_t x = 0; x < UniformGrid3D::SIZE; x++)
    //   {
    //     uint32_t &column = grid.getColumn(x, 0, z);
    //     uint32_t first = column & ~(column << 1);
    //     uint32_t last = column & ~(column >> 1);

    //     while (first)
    //     {
    //       unsigned int w = __builtin_ffs(first) - 1;

    //       unsigned int wi = x + (UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * z)));
    //       unsigned int hi = z + (UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * x)));

    //       wMask[wi / UniformGrid3D::BITS] |= (1U << (wi % UniformGrid3D::BITS));
    //       hMask[hi / UniformGrid3D::BITS] |= (1U << (hi % UniformGrid3D::BITS));

    //       // generateFace(vertices, {x, w, z}, {1.0f, 1.0f, 1.0f}, FaceDirection::BOTTOM);
    //       first &= ~((1UL << w + 1) - 1);
    //     }
    //   }
    // }

    // for (size_t z = 0; z < UniformGrid3D::SIZE; z++)
    // {
    //   for (size_t x = 0; x < UniformGrid3D::SIZE; x++)
    //   {

    //     uint8_t &column = grid.getColumn(x, 0, z);
    //     uint8_t first = column & ~(column << 1);
    //     uint8_t last = column & ~(column >> 1);

    //     while (first)
    //     {
    //       unsigned int w = __builtin_ffs(first) - 1;

    //       uint32_t &width = wMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * z))) / UniformGrid3D::BITS];
    //       uint32_t &height = hMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * x))) / UniformGrid3D::BITS];

    //       if (!width || !height)
    //       {
    //         first &= ~((1UL << w + 1) - 1);
    //         continue;
    //       }

    //       std::cout << "c " << std::bitset<32>(first) << std::endl;
    //       std::cout << "w " << std::bitset<32>(width) << std::endl;
    //       std::cout << "h " << std::bitset<32>(height) << std::endl;

    //       const Info wInfo = getInfo(width);
    //       const Info hInfo = getInfo(height);

    //       // generateFace(vertices, {x, w, z}, {wInfo.size, 1.0f, 1.0f}, FaceDirection::BOTTOM);
    //       generateFace(vertices, {x, w, z}, {wInfo.size, 1.0f, hInfo.size}, FaceDirection::BOTTOM);

    //       first &= ~((1UL << w + 1) - 1);
    //       width &= ~((1UL << (wInfo.offset + (int)wInfo.size)) - 1);
    //       height &= ~((1UL << (hInfo.offset + (int)hInfo.size)) - 1);
    //     }
    //   }
    // }

    // for (size_t z = 0; z < UniformGrid3D::SIZE; z++)
    // {
    //   for (size_t x = 0; x < UniformGrid3D::SIZE; x++)
    //   {
    //     for (size_t y = 0; y < UniformGrid3D::SIZE; y++)
    //     {
    //       if (lMask[x][y][z])
    //       {
    //         // Try to extend the top face in the x and z directions
    //         int width = 1;
    //         while (x + width < UniformGrid3D::SIZE && lMask[x + width][y][z])
    //         {
    //           width++;
    //         }

    //         int depth = 1;
    //         while (z + depth < UniformGrid3D::SIZE && lMask[x][y][z + depth])
    //         {
    //           depth++;
    //         }

    //         // Now we have the width and depth. Create the face.
    //         generateFace(vertices, {x, y, z}, {width, 1.0f, depth}, FaceDirection::TOP);

    //         // Clear the used mask entries (important!)
    //         for (int i = 0; i < width; i++)
    //         {
    //           for (int j = 0; j < depth; j++)
    //           {
    //             lMask[x + i][y][z + j] = false;
    //           }
    //         }
    //       }

    //       // Same for fMask
    //       if (fMask[x][y][z])
    //       {
    //         // Try to extend the bottom face in the x and z directions
    //         int width = 1;
    //         while (x + width < UniformGrid3D::SIZE && fMask[x + width][y][z])
    //         {
    //           width++;
    //         }

    //         int depth = 1;
    //         while (z + depth < UniformGrid3D::SIZE && fMask[x][y][z + depth])
    //         {
    //           depth++;
    //         }

    //         // Now we have the width and depth. Create the face.
    //         generateFace(vertices, {x, y, z}, {width, 1.0f, depth}, FaceDirection::BOTTOM);

    //         // Clear the used mask entries (important!)
    //         for (int i = 0; i < width; i++)
    //         {
    //           for (int j = 0; j < depth; j++)
    //           {
    //             fMask[x + i][y][z + j] = false;
    //           }
    //         }
    //       }
    //     }
    //   }
    // }

    // for (size_t z = 0; z < UniformGrid3D::SIZE; z++)
    // {
    //   for (size_t x = 0; x < UniformGrid3D::SIZE; x++)
    //   {
    //     uint32_t &column = grid.getColumn(x, 0, z);
    //     uint32_t first = column & ~(column << 1);
    //     uint32_t last = column & ~(column >> 1);

    //     while (first)
    //     {
    //       unsigned int w = __builtin_ffs(first) - 1;
    //       generateFace(vertices, {x, w, z}, {1.0f, 1.0f, 1.0f}, FaceDirection::BOTTOM);
    //       first &= ~((1UL << w + 1) - 1);
    //     }

    //     while (last)
    //     {
    //       unsigned int w = __builtin_ffs(last) - 1;
    //       generateFace(vertices, {x, w, z}, {1.0f, 1.0f, 1.0f}, FaceDirection::TOP);
    //       last &= ~((1UL << w + 1) - 1);
    //     }
    //   }
    // }

    // for (size_t z = 0; z < size.z; z++)
    // {
    //   for (size_t y = 0; y < size.y; y++)
    //   {
    //     uint32_t &row = grid.getRow(0, y, z);
    //     uint32_t first = row & ~(row << 1);
    //     uint32_t last = row & ~(row >> 1);

    //     while (first)
    //     {
    //       unsigned int w = __builtin_ffs(first) - 1;
    //       generateFace(vertices, {w, y, z}, {1.0f, 1.0f, 1.0f}, FaceDirection::LEFT);
    //       first &= ~((1UL << w + 1) - 1);
    //     }

    //     while (last)
    //     {
    //       unsigned int w = __builtin_ffs(last) - 1;
    //       generateFace(vertices, {w, y, z}, {1.0f, 1.0f, 1.0f}, FaceDirection::RIGHT);
    //       last &= ~((1UL << w + 1) - 1);
    //     }
    //   }
    // }

    // for (size_t x = 0; x < size.x; x++)
    // {
    //   for (size_t y = 0; y < size.y; y++)
    //   {
    //     uint32_t &depth = grid.getLayer(x, y, 0);
    //     uint32_t first = depth & ~(depth << 1);
    //     uint32_t last = depth & ~(depth >> 1);

    //     while (first)
    //     {
    //       unsigned int w = __builtin_ffs(first) - 1;
    //       generateFace(vertices, {x, y, w}, {1.0f, 1.0f, 0.0f}, FaceDirection::FRONT);
    //       first &= ~((1UL << w + 1) - 1);
    //     }

    //     while (last)
    //     {
    //       unsigned int w = __builtin_ffs(last) - 1;
    //       generateFace(vertices, {x, y, w}, {1.0f, 1.0f, 1.0f}, FaceDirection::BACK);
    //       last &= ~((1UL << w + 1) - 1);
    //     }
    //   }
    // }

    // for (size_t z = 0; z < size.z; z++)
    // {
    //   for (size_t x = 0; x < size.x; x++)
    //   {
    //     uint32_t &column = voxels.getColumn(x, 0, z);
    //     unsigned int offset = __builtin_ffs(column) - 1;

    //     bool alt = false;

    //     while (column)
    //     {
    //       const Info iCol = getInfo(column);
    //       // generateFace(vertices, {x, iCol.offset, z}, {iaRow.size, 1.0f, 1.0f}, FaceDirection::TOP);
    //       column &= voxels.createMask(iCol.size + iCol.offset);
    //       alt = !alt;
    //     }
    //   }
    // }

    // std::cout << vertices.size() << std::endl;

    // /**
    //  * Perf
    //  * Circle 1.01195, 1.00715, 1.00628, 1.00891, 1.00919
    //  * Cube 0.317164, 0.317527, 0.312736, 0.312611, 0.319995
    //  */
    // for (size_t z = 0; z < size.z; z++)
    // {
    //   for (size_t x = 0; x < size.x; x++)
    //   {
    //     uint32_t &column = voxels.getColumn(x, 0, z);

    //     while (column)
    //     {
    //       Info iCol = getInfo(column);
    //       column &= voxels.createMask(iCol.size + iCol.offset);

    //       glm::vec3 position(x, iCol.offset, z);
    //       glm::vec3 size(1.0f, iCol.size, 1.0f);

    //       a.set(x, iCol.size + iCol.offset - 1.0f, z, 1);
    //       b.set(x, iCol.offset, z, 1);
    //     }
    //   }
    // }

    // for (size_t z = 0; z < size.z; z++)
    // {
    //   for (size_t x = 0; x < size.x; x++)
    //   {
    //     uint32_t &ca = a.getColumn(x, 0, z);
    //     uint32_t &cb = b.getColumn(x, 0, z);

    //     while (ca || cb)
    //     {
    //       const Info ica = getInfo(ca);
    //       uint32_t &arow = a.getRow(x, ica.size + ica.offset - 1.0f, z);

    //       const Info icb = getInfo(cb);
    //       uint32_t &brow = b.getRow(x, icb.offset, z);

    //       while (arow)
    //       {
    //         const Info iaRow = getInfo(arow);
    //         arow &= a.createMask(iaRow.size + iaRow.offset);
    //         generateFace(vertices, {iaRow.offset, ica.size + ica.offset - 1.0f, z}, {iaRow.size, 1.0f, 1.0f}, FaceDirection::TOP);
    //       }

    //       while (brow)
    //       {
    //         const Info ibRow = getInfo(brow);
    //         brow &= a.createMask(ibRow.size + ibRow.offset);
    //         generateFace(vertices, {ibRow.offset, icb.offset, z}, {ibRow.size, 1.0f, 1.0f}, FaceDirection::BOTTOM);
    //       }

    //       ca &= a.createMask(ica.size + ica.offset);
    //       cb &= b.createMask(icb.size + icb.offset);
    //     }
    //   }
    // }

    // a.clear();
    // b.clear();

    // for (size_t z = 0; z < size.z; z++)
    // {
    //   for (size_t y = 0; y < size.y; y++)
    //   {
    //     uint32_t &row = voxels.getRow(0, y, z);

    //     while (row)
    //     {
    //       Info iRow = getInfo(row);
    //       row &= voxels.createMask(iRow.size + iRow.offset);

    //       a.set(iRow.size + iRow.offset - 1.0f, y, z, 1);
    //       b.set(iRow.offset, y, z, 1);
    //     }
    //   }
    // }

    // for (size_t z = 0; z < size.z; z++)
    // {
    //   for (size_t y = 0; y < size.y; y++)
    //   {
    //     uint32_t &ra = a.getRow(0, y, z);
    //     uint32_t &rb = b.getRow(0, y, z);

    //     while (ra || rb)
    //     {
    //       const Info ira = getInfo(ra);
    //       uint32_t &acol = a.getColumn(ira.size + ira.offset - 1.0f, y, z);

    //       const Info irb = getInfo(rb);
    //       uint32_t &bcol = b.getColumn(irb.offset, y, z);

    //       while (acol)
    //       {
    //         const Info iaCol = getInfo(acol);
    //         acol &= a.createMask(iaCol.size + iaCol.offset);
    //         generateFace(vertices, {ira.size + ira.offset  - 1.0f, iaCol.offset, z}, {1.0f, iaCol.size, 1.0f}, FaceDirection::RIGHT);
    //       }

    //       while (bcol)
    //       {
    //         const Info ibCol = getInfo(bcol);
    //         bcol &= a.createMask(ibCol.size + ibCol.offset);
    //         generateFace(vertices, {irb.offset, ibCol.offset, z}, {1.0f, ibCol.size, 1.0f}, FaceDirection::LEFT);
    //       }

    //       ra &= a.createMask(ira.size + ira.offset);
    //       rb &= b.createMask(irb.size + irb.offset);
    //     }
    //   }
    // }

    // a.clear();
    // b.clear();

    // for (size_t x = 0; x < size.x; x++)
    // {
    //   for (size_t y = 0; y < size.y; y++)
    //   {
    //     uint32_t &depth = voxels.getLayer(x, y, 0);

    //     while (depth)
    //     {
    //       Info iDepth = getInfo(depth);
    //       depth &= voxels.createMask(iDepth.size + iDepth.offset);

    //       a.set(x, y, iDepth.size + iDepth.offset - 1.0f, 1);
    //       b.set(x, y, iDepth.offset, 1);
    //     }
    //   }
    // }

    // for (size_t x = 0; x < size.x; x++)
    // {
    //   for (size_t y = 0; y < size.y; y++)
    //   {
    //     uint32_t &da = a.getLayer(x, y, 0);
    //     uint32_t &db = b.getLayer(x, y, 0);

    //     while (da || db)
    //     {
    //       const Info ida = getInfo(da);
    //       uint32_t &arow = a.getRow(x, y, ida.size + ida.offset - 1.0f);

    //       const Info idb = getInfo(db);
    //       uint32_t &brow = b.getRow(x, y, idb.offset);

    //       while (arow)
    //       {
    //         const Info iaRow = getInfo(arow);
    //         arow &= a.createMask(iaRow.size + iaRow.offset);
    //         generateFace(vertices, {iaRow.offset, y, ida.size + ida.offset - 1.0f}, {iaRow.size, 1.0f, 1.0f}, FaceDirection::BACK);
    //       }

    //       while (brow)
    //       {
    //         const Info ibRow = getInfo(brow);
    //         brow &= a.createMask(ibRow.size + ibRow.offset);
    //         generateFace(vertices, {ibRow.offset, y, idb.offset}, {ibRow.size, 1.0f, 1.0f}, FaceDirection::FRONT);
    //       }

    //       da &= a.createMask(ida.size + ida.offset);
    //       db &= b.createMask(idb.size + ida.offset);
    //     }
    //   }
    // }

    // /**
    //  * Perf
    //  * Circle 1.25888, 1.26835, 1.25914, 1.27016, 1.25388
    //  * Cube 1.58843, 1.5937, 1.5932, 1.60254, 1.5867
    //  */
    // for (size_t z = 0; z < size.z; z++)
    // {
    //   for (size_t x = 0; x < size.x; x++)
    //   {
    //     uint32_t &column = voxels.getColumn(x, 0, z);

    //     while (column)
    //     {
    //       Info iCol = getInfo(column);
    //       column &= voxels.createMask(iCol.size + iCol.offset);

    //       glm::vec3 position(x, iCol.offset, z);
    //       glm::vec3 size(1.0f, iCol.size, 1.0f);

    //       generateFace(vertices, position, size, FaceDirection::TOP);
    //       generateFace(vertices, position, size, FaceDirection::BOTTOM);
    //     }
    //   }
    // }

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

    //       generateFace(vertices, position, size, FaceDirection::LEFT);
    //       generateFace(vertices, position, size, FaceDirection::RIGHT);
    //     }
    //   }
    // }

    // for (size_t x = 0; x < size.x; x++)
    // {
    //   for (size_t y = 0; y < size.y; y++)
    //   {
    //     uint32_t &depth = voxels.getLayer(x, y, 0);

    //     while (depth)
    //     {
    //       Info iDepth = getInfo(depth);
    //       depth &= voxels.createMask(iDepth.size + iDepth.offset);

    //       glm::vec3 position(x, y, iDepth.offset);
    //       glm::vec3 size(1.0f, 1.0f, iDepth.size);

    //       generateFace(vertices, position, size, FaceDirection::FRONT);
    //       generateFace(vertices, position, size, FaceDirection::BACK);
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
          grid.set(x, y, z, 1);
      }
    }
  }

  void fill(const glm::ivec3 &size)
  {
    for (size_t z = 0; z < size.z; z++)
      for (size_t x = 0; x < size.x; x++)
        for (size_t y = 0; y < size.y; y++)
          grid.set(x, y, z, 1);
  }

  void fillPlane(const glm::ivec3 &size)
  {
    for (size_t z = 0; z < size.z; z++)
      for (size_t x = 0; x < size.x; x++)
        grid.set(x, 0, z, 1);
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
            grid.set(x, y, z, 1);
        }
  }
};