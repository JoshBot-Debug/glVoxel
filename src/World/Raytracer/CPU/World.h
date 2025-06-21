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

namespace RaytracerCPU {

class World {
private:
  Registry *m_Registry = nullptr;

  VoxelManager m_Voxels;

  PerspectiveCamera *m_Camera = nullptr;

  Texture2D texture;

public:
  HeightMap heightMap{128, 128};

public:
  World();

  void initialize();

  void draw();

  void update();

  void setRegistry(Registry *registry);

  void setCamera(PerspectiveCamera *camera);
};

}; // namespace RaytracerCPU