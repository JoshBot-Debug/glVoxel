#pragma once

#include "World/Octree/SparseVoxelOctree.h"
#include "Engine/Core/Buffer.h"
#include "Engine/Core/VertexArray.h"
#include "Engine/Texture2D.h"
#include "Engine/Camera/PerspectiveCamera.h"

#include <noise/noise.h>
#include <noise/noiseutils.h>

struct Terrain
{
  float frequency = 1.0;
  float persistence = 0.4;
  int octaveCount = 3.0;
  int seed = 50;

  float scale = 0.6;
  float bias = -0.4;

  int destWidth;
  int destHeight;

  float stoneThreshold = 0.20f;
  float dirtThreshold = 0.30f;
  float grassThreshold = 0.5f;

  Terrain(int destWidth, int destHeight) : destWidth(destWidth), destHeight(destHeight) {}
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

  SparseVoxelOctree tree{256};
  std::vector<Vertex> vertices;

  PerspectiveCamera *camera = nullptr;

  noise::module::Perlin perlin;
  noise::module::ScaleBias scaleBias;
  utils::NoiseMapBuilderPlane heightMapBuilder;
  utils::NoiseMap heightMap;

public:
  Terrain terrain{tree.getSize(), tree.getSize()};
  DrawMode drawMode = DrawMode::TRIANGLES;

public:
  World();

  void draw();

  void setBuffer();

  void generateChunk(int worldX, int worldZ);

  void fill();

  void fillSphere();

  void setCamera(PerspectiveCamera *camera);

  void initialize();
};