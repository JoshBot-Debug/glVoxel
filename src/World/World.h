#pragma once

#include "World/Octree/SparseVoxelOctree.h"
#include "World/Octree/VoxelManager.h"
#include "Engine/Core/Buffer.h"
#include "Engine/Core/VertexArray.h"
#include "Engine/Texture2D.h"
#include "Engine/Camera/PerspectiveCamera.h"
#include "HeightMap.h"

#include <noise/noise.h>
#include <noise/noiseutils.h>

enum class DrawMode : GLenum
{
  TRIANGLES = GL_TRIANGLES,
  LINES = GL_LINES,
};

class World
{
private:
  Buffer vbo{BufferTarget::ARRAY_BUFFER, VertexDraw::DYNAMIC};
  VertexArray vao;

  VoxelManager voxels{128, 1};

  PerspectiveCamera *camera = nullptr;


public:
  HeightMap heightMap{128, 128};
  DrawMode drawMode = DrawMode::TRIANGLES;

public:
  void initialize();
  
  void draw();

  void update();

  // Private in the future
  void setBuffer();

  void setCamera(PerspectiveCamera *camera);
};