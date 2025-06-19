#pragma once

#include "ECS/Entity.h"

#include "Voxel/HeightMap.h"
#include "Voxel/SparseVoxelOctree.h"

#include "Engine/Camera/PerspectiveCamera.h"
#include "Engine/Core/Buffer.h"
#include "Engine/Core/TripleBuffer.h"
#include "Engine/Core/VertexArray.h"
#include "Engine/Texture2D.h"

#include "VoxelManager.h"

#include <mutex>
#include <noise/noise.h>
#include <noise/noiseutils.h>

enum class DrawMode : GLenum {
  TRIANGLES = GL_TRIANGLES,
  LINES = GL_LINES,
};

class World {
private:
  Registry *m_Registry{nullptr};

  TripleBuffer<BufferTarget::ARRAY_BUFFER, VertexDraw::DYNAMIC> m_Buffer;

  VoxelManager m_Voxels;

  PerspectiveCamera *m_Camera = nullptr;

public:
  HeightMap heightMap{256, 256};
  DrawMode drawMode = DrawMode::TRIANGLES;

public:
  World();

  void initialize();

  void draw();

  void update();

  void setRegistry(Registry *registry);

  void setCamera(PerspectiveCamera *camera);
};