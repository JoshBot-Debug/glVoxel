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

    // grid.set(3, 0, 1, 1);
    // grid.set(4, 0, 1, 1);
    // grid.set(5, 0, 1, 1);
    // grid.set(6, 0, 1, 1);
    // grid.set(4, 0, 2, 1);
    // grid.set(5, 0, 2, 1);
    // grid.set(3, 0, 3, 1);
    // grid.set(4, 0, 3, 1);
    // grid.set(5, 0, 3, 1);
    // grid.set(6, 0, 3, 1);

    // grid.set(4, 0, 0, 1);
    // grid.set(5, 1, 0, 1);

    // grid.set(3, 1, 1, 1);
    // grid.set(4, 1, 1, 1);
    // grid.set(5, 1, 1, 1);
    // grid.set(6, 2, 1, 1);

    // grid.set(3, 1, 2, 1);
    // grid.set(5, 1, 2, 1);

    grid.set(9, 0, 0, 1);
    grid.set(10, 0, 0, 1);
    grid.set(8, 0, 1, 1);
    grid.set(9, 0, 1, 1);
    grid.set(10, 0, 1, 1);
    grid.set(11, 0, 1, 1);
    grid.set(9, 0, 2, 1);
    grid.set(11, 0, 2, 1);

    // generateNoise();
    // fillSphere(grid.size());
    // fill(grid.size());

    update();
    setBuffer();
  }

  void draw()
  {
    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glDrawArrays(GL_LINES, 0, vertices.size());
  }

  void update()
  {
    vertices.clear();

    uint32_t wfMask[UniformGrid3D::GRID_SIZE] = {};
    uint32_t hfMask[UniformGrid3D::GRID_SIZE] = {};

    uint32_t wlMask[UniformGrid3D::GRID_SIZE] = {};
    uint32_t hlMask[UniformGrid3D::GRID_SIZE] = {};

    for (size_t z = 0; z < UniformGrid3D::SIZE; z++)
    {
      for (size_t x = 0; x < UniformGrid3D::SIZE; x++)
      {
        uint32_t &column = grid.getColumn(x, 0, z);
        uint32_t first = column & ~(column << 1);
        uint32_t last = column & ~(column >> 1);

        while (first)
        {
          unsigned int w = __builtin_ffs(first) - 1;

          unsigned int wi = x + (UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * z)));
          wfMask[wi / UniformGrid3D::BITS] |= (1U << (wi % UniformGrid3D::BITS));

          unsigned int hi = z + (UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * x)));
          hfMask[hi / UniformGrid3D::BITS] |= (1U << (hi % UniformGrid3D::BITS));

          first &= ~((1UL << w + 1) - 1);
        }

        // while (last)
        // {
        //   unsigned int w = __builtin_ffs(last) - 1;

        //   unsigned int wi = x + (UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * z)));
        //   wlMask[wi / UniformGrid3D::BITS] |= (1U << (wi % UniformGrid3D::BITS));

        //   unsigned int hi = z + (UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * x)));
        //   hlMask[hi / UniformGrid3D::BITS] |= (1U << (hi % UniformGrid3D::BITS));

        //   last &= ~((1UL << w + 1) - 1);
        // }
      }
    }

    for (size_t z = 0; z < UniformGrid3D::SIZE; z++)
    {
      for (size_t x = 0; x < UniformGrid3D::SIZE; x++)
      {
        uint32_t &column = grid.getColumn(x, 0, z);
        uint32_t first = column & ~(column << 1);
        uint32_t last = column & ~(column >> 1);

        while (first)
        {
          unsigned int w = __builtin_ffs(first) - 1;
          first &= ~((1UL << w + 1) - 1);

          uint32_t &width = wfMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * z))) / UniformGrid3D::BITS] &= ~((1UL << x) - 1);
          if(!width) continue;

          const Info wInfo = getInfo(width);

          uint32_t &height = hfMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * (int)(wInfo.offset)))) / UniformGrid3D::BITS] &= ~((1UL << z) - 1);

          const Info hInfo = getInfo(height);

          std::cout << "x:y:z " << x << " " << w << " " << z << std::endl;
          std::cout << "wo:ho " << wInfo.offset << " " << hInfo.offset << std::endl;
          std::cout << "ws:hs " << wInfo.size << " " << hInfo.size << std::endl;
          std::cout << "c " << std::bitset<32>(column) << std::endl;
          std::cout << "h " << std::bitset<32>(height) << std::endl;
          std::cout << "w " << std::bitset<32>(width) << std::endl;

          unsigned int faceHeight = 1;

          for (size_t z1 = hInfo.offset + 1; z1 < hInfo.offset + hInfo.size; z1++)
          {
            uint32_t w1 = wfMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * z1))) / UniformGrid3D::BITS];

            w1 &= ((1UL << (int)wInfo.size) - 1) << wInfo.offset;

            if (getInfo(w1).size == wInfo.size)
            {
              ++faceHeight;
              uint32_t &w11 = wfMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * z1))) / UniformGrid3D::BITS];
              w11 &= w11 ^ (((1UL << (int)wInfo.size) - 1) << wInfo.offset);
            }
          }

          generateFace(vertices, {wInfo.offset, w, z}, {wInfo.size, 1.0f, faceHeight}, FaceDirection::BOTTOM);

          width &= ~((1UL << (wInfo.offset + (int)wInfo.size)) - 1);
          height &= ~((1UL << (z + (int)faceHeight)) - 1);
        }

        // while (last)
        // {
        //   unsigned int w = __builtin_ffs(last) - 1;
        //   last &= ~((1UL << w + 1) - 1);

        //   uint32_t &width = wlMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * z))) / UniformGrid3D::BITS];
        //   uint32_t &height = hlMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * x))) / UniformGrid3D::BITS];

        //   height &= ~((1UL << z) - 1);
        //   width &= ~((1UL << x) - 1);

        //   if (!width || !height)
        //     continue;

        //   const Info wInfo = getInfo(width);
        //   const Info hInfo = getInfo(height);

        //   for (size_t z1 = hInfo.offset; z1 < hInfo.offset + hInfo.size; z1++)
        //   {
        //     uint32_t &w1 = wlMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * z1))) / UniformGrid3D::BITS];
        //     w1 &= w1 ^ (((1UL << (int)wInfo.size) - 1) << wInfo.offset);
        //   }

        //   for (size_t x1 = wInfo.offset; x1 < wInfo.offset + wInfo.size; x1++)
        //   {
        //     uint32_t &h1 = hlMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * x1))) / UniformGrid3D::BITS];
        //     h1 &= h1 ^ (((1UL << (int)hInfo.size) - 1) << hInfo.offset);
        //   }

        //   generateFace(vertices, {wInfo.offset, w, hInfo.offset}, {wInfo.size, 1.0f, hInfo.size}, FaceDirection::TOP);

        //   width &= ~((1UL << (wInfo.offset + (int)wInfo.size)) - 1);
        //   height &= ~((1UL << (hInfo.offset + (int)hInfo.size)) - 1);
        // }
      }
    }

    // std::memset(wfMask, 0, sizeof(wfMask));
    // std::memset(hfMask, 0, sizeof(hfMask));
    // std::memset(wlMask, 0, sizeof(wlMask));
    // std::memset(hlMask, 0, sizeof(hlMask));

    // for (size_t z = 0; z < UniformGrid3D::SIZE; z++)
    // {
    //   for (size_t y = 0; y < UniformGrid3D::SIZE; y++)
    //   {
    //     uint32_t &row = grid.getRow(0, y, z);
    //     uint32_t first = row & ~(row << 1);
    //     uint32_t last = row & ~(row >> 1);

    //     while (first)
    //     {
    //       unsigned int w = __builtin_ffs(first) - 1;

    //       unsigned int wi = y + (UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * z)));
    //       wfMask[wi / UniformGrid3D::BITS] |= (1U << (wi % UniformGrid3D::BITS));

    //       unsigned int hi = z + (UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * y)));
    //       hfMask[hi / UniformGrid3D::BITS] |= (1U << (hi % UniformGrid3D::BITS));

    //       first &= ~((1UL << w + 1) - 1);
    //     }

    //     while (last)
    //     {
    //       unsigned int w = __builtin_ffs(last) - 1;

    //       unsigned int wi = y + (UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * z)));
    //       wlMask[wi / UniformGrid3D::BITS] |= (1U << (wi % UniformGrid3D::BITS));

    //       unsigned int hi = z + (UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * y)));
    //       hlMask[hi / UniformGrid3D::BITS] |= (1U << (hi % UniformGrid3D::BITS));

    //       last &= ~((1UL << w + 1) - 1);
    //     }
    //   }
    // }

    // for (size_t z = 0; z < UniformGrid3D::SIZE; z++)
    // {
    //   for (size_t y = 0; y < UniformGrid3D::SIZE; y++)
    //   {
    //     uint32_t &row = grid.getRow(0, y, z);
    //     uint32_t first = row & ~(row << 1);
    //     uint32_t last = row & ~(row >> 1);

    //     while (first)
    //     {
    //       unsigned int w = __builtin_ffs(first) - 1;
    //       first &= ~((1UL << w + 1) - 1);

    //       uint32_t &width = wfMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * z))) / UniformGrid3D::BITS];
    //       uint32_t &height = hfMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * y))) / UniformGrid3D::BITS];

    //       height &= ~((1UL << z) - 1);
    //       width &= ~((1UL << y) - 1);

    //       if (!width || !height)
    //         continue;

    //       const Info wInfo = getInfo(width);
    //       const Info hInfo = getInfo(height);

    //       for (size_t z1 = hInfo.offset; z1 < hInfo.offset + hInfo.size; z1++)
    //       {
    //         uint32_t &w1 = wfMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * z1))) / UniformGrid3D::BITS];
    //         w1 &= w1 ^ (((1UL << (int)wInfo.size) - 1) << wInfo.offset);
    //       }

    //       for (size_t y1 = wInfo.offset; y1 < wInfo.offset + wInfo.size; y1++)
    //       {
    //         uint32_t &h1 = hfMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * y1))) / UniformGrid3D::BITS];
    //         h1 &= h1 ^ (((1UL << (int)hInfo.size) - 1) << hInfo.offset);
    //       }

    //       generateFace(vertices, {w, wInfo.offset, hInfo.offset}, {1.0f, wInfo.size, hInfo.size}, FaceDirection::LEFT);

    //       width &= ~((1UL << (wInfo.offset + (int)wInfo.size)) - 1);
    //       height &= ~((1UL << (hInfo.offset + (int)hInfo.size)) - 1);
    //     }

    //     while (last)
    //     {
    //       unsigned int w = __builtin_ffs(last) - 1;
    //       last &= ~((1UL << w + 1) - 1);

    //       uint32_t &width = wlMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * z))) / UniformGrid3D::BITS];
    //       uint32_t &height = hlMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * y))) / UniformGrid3D::BITS];

    //       height &= ~((1UL << z) - 1);
    //       width &= ~((1UL << y) - 1);

    //       if (!width || !height)
    //         continue;

    //       const Info wInfo = getInfo(width);
    //       const Info hInfo = getInfo(height);

    //       for (size_t z1 = hInfo.offset; z1 < hInfo.offset + hInfo.size; z1++)
    //       {
    //         uint32_t &w1 = wlMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * z1))) / UniformGrid3D::BITS];
    //         w1 &= w1 ^ (((1UL << (int)wInfo.size) - 1) << wInfo.offset);
    //       }

    //       for (size_t y1 = wInfo.offset; y1 < wInfo.offset + wInfo.size; y1++)
    //       {
    //         uint32_t &h1 = hlMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * y1))) / UniformGrid3D::BITS];
    //         h1 &= h1 ^ (((1UL << (int)hInfo.size) - 1) << hInfo.offset);
    //       }

    //       generateFace(vertices, {w, wInfo.offset, hInfo.offset}, {1.0f, wInfo.size, hInfo.size}, FaceDirection::RIGHT);

    //       width &= ~((1UL << (wInfo.offset + (int)wInfo.size)) - 1);
    //       height &= ~((1UL << (hInfo.offset + (int)hInfo.size)) - 1);
    //     }
    //   }
    // }

    // std::memset(wfMask, 0, sizeof(wfMask));
    // std::memset(hfMask, 0, sizeof(hfMask));
    // std::memset(wlMask, 0, sizeof(wlMask));
    // std::memset(hlMask, 0, sizeof(hlMask));

    // for (size_t x = 0; x < UniformGrid3D::SIZE; x++)
    // {
    //   for (size_t y = 0; y < UniformGrid3D::SIZE; y++)
    //   {
    //     uint32_t &depth = grid.getLayer(x, y, 0);
    //     uint32_t first = depth & ~(depth << 1);
    //     uint32_t last = depth & ~(depth >> 1);

    //     while (first)
    //     {
    //       unsigned int w = __builtin_ffs(first) - 1;

    //       unsigned int wi = y + (UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * x)));
    //       wfMask[wi / UniformGrid3D::BITS] |= (1U << (wi % UniformGrid3D::BITS));

    //       unsigned int hi = x + (UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * y)));
    //       hfMask[hi / UniformGrid3D::BITS] |= (1U << (hi % UniformGrid3D::BITS));

    //       first &= ~((1UL << w + 1) - 1);
    //     }

    //     while (last)
    //     {
    //       unsigned int w = __builtin_ffs(last) - 1;

    //       unsigned int wi = y + (UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * x)));
    //       wlMask[wi / UniformGrid3D::BITS] |= (1U << (wi % UniformGrid3D::BITS));

    //       unsigned int hi = x + (UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * y)));
    //       hlMask[hi / UniformGrid3D::BITS] |= (1U << (hi % UniformGrid3D::BITS));

    //       last &= ~((1UL << w + 1) - 1);
    //     }
    //   }
    // }

    // for (size_t x = 0; x < UniformGrid3D::SIZE; x++)
    // {
    //   for (size_t y = 0; y < UniformGrid3D::SIZE; y++)
    //   {
    //     uint32_t &depth = grid.getLayer(x, y, 0);
    //     uint32_t first = depth & ~(depth << 1);
    //     uint32_t last = depth & ~(depth >> 1);

    //     while (first)
    //     {
    //       unsigned int w = __builtin_ffs(first) - 1;
    //       first &= ~((1UL << w + 1) - 1);

    //       uint32_t &width = wfMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * x))) / UniformGrid3D::BITS];
    //       uint32_t &height = hfMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * y))) / UniformGrid3D::BITS];

    //       height &= ~((1UL << x) - 1);
    //       width &= ~((1UL << y) - 1);

    //       if (!width || !height)
    //         continue;

    //       const Info wInfo = getInfo(width);
    //       const Info hInfo = getInfo(height);

    //       for (size_t x1 = hInfo.offset; x1 < hInfo.offset + hInfo.size; x1++)
    //       {
    //         uint32_t &w1 = wfMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * x1))) / UniformGrid3D::BITS];
    //         w1 &= w1 ^ (((1UL << (int)wInfo.size) - 1) << wInfo.offset);
    //       }

    //       for (size_t y1 = wInfo.offset; y1 < wInfo.offset + wInfo.size; y1++)
    //       {
    //         uint32_t &h1 = hfMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * y1))) / UniformGrid3D::BITS];
    //         h1 &= h1 ^ (((1UL << (int)hInfo.size) - 1) << hInfo.offset);
    //       }

    //       generateFace(vertices, {wInfo.offset, hInfo.offset, w}, {wInfo.size, hInfo.size, 1.0f}, FaceDirection::FRONT);

    //       width &= ~((1UL << (wInfo.offset + (int)wInfo.size)) - 1);
    //       height &= ~((1UL << (hInfo.offset + (int)hInfo.size)) - 1);
    //     }

    //     while (last)
    //     {
    //       unsigned int w = __builtin_ffs(last) - 1;
    //       last &= ~((1UL << w + 1) - 1);

    //       uint32_t &width = wlMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * x))) / UniformGrid3D::BITS];
    //       uint32_t &height = hlMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * y))) / UniformGrid3D::BITS];

    //       height &= ~((1UL << x) - 1);
    //       width &= ~((1UL << y) - 1);

    //       if (!width || !height)
    //         continue;

    //       const Info wInfo = getInfo(width);
    //       const Info hInfo = getInfo(height);

    //       for (size_t x1 = hInfo.offset; x1 < hInfo.offset + hInfo.size; x1++)
    //       {
    //         uint32_t &w1 = wlMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * x1))) / UniformGrid3D::BITS];
    //         w1 &= w1 ^ (((1UL << (int)wInfo.size) - 1) << wInfo.offset);
    //       }

    //       for (size_t y1 = wInfo.offset; y1 < wInfo.offset + wInfo.size; y1++)
    //       {
    //         uint32_t &h1 = hlMask[(UniformGrid3D::SIZE * (w + (UniformGrid3D::SIZE * y1))) / UniformGrid3D::BITS];
    //         h1 &= h1 ^ (((1UL << (int)hInfo.size) - 1) << hInfo.offset);
    //       }

    //       generateFace(vertices, {wInfo.offset, hInfo.offset, w}, {wInfo.size, hInfo.size, 1.0f}, FaceDirection::BACK);

    //       width &= ~((1UL << (wInfo.offset + (int)wInfo.size)) - 1);
    //       height &= ~((1UL << (hInfo.offset + (int)hInfo.size)) - 1);
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