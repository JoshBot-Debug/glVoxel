#pragma once

#include "World/SparseVoxelOctree.h"
#include "Engine/Core/Buffer.h"
#include "Engine/Core/VertexArray.h"

struct TerrainProperties
{
  float lowerXBound = 0.0;
  float upperXBound = 2.0;
  float lowerZBound = 0.0;
  float upperZBound = 2.0;

  float frequency = 1.0;
  float persistence = 0.4;
  int octaveCount = 3.0;
  int maxHeight = 0;
  int seed = 0;

  float scale = 0.6;
  float bias = -0.4;

  int destWidth;
  int destHeight;

  float stoneThreshold = 0.20f;
  float dirtThreshold = 0.30f;
  float grassThreshold = 0.5f;

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
  Voxel::SparseVoxelOctree tree{256};
  std::vector<Vertex> vertices;

public:
  TerrainProperties terrain{tree.getSize(), tree.getSize()};
  DrawMode drawMode = DrawMode::TRIANGLES;

public:
  World();

  void draw();

  void setBuffer();

  const Voxel::SparseVoxelOctree &getTree() const;

  void generateTerrain();

  void fill();

  void fillSphere();
};