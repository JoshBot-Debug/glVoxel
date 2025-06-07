#pragma once

#include "World/SparseVoxelOctree.h"
#include "Engine/Core/Buffer.h"
#include "Engine/Core/VertexArray.h"

struct TerrainProperties
{
  double lowerXBound = 1.0;
  double upperXBound = 2.0;
  double lowerZBound = 1.0;
  double upperZBound = 2.0;

  int destWidth;
  int destHeight;

  TerrainProperties(int destWidth, int destHeight) : destWidth(destWidth), destHeight(destHeight) {}
};

enum class DrawMode : GLenum
{
  TRIANGLES = GL_TRIANGLES,
  LINES = GL_LINES,
};

class World
{
private:
  Buffer vbo;
  VertexArray vao;
  Voxel::SparseVoxelOctree tree{128};
  std::vector<Vertex> vertices;

public:
  TerrainProperties terrain{tree.getSize(), tree.getSize()};
  DrawMode drawMode = DrawMode::TRIANGLES;

public:
  World();

  void draw();

  void setBuffer();

  void generateTerrain();

  void fill();

  void fillSphere();
};