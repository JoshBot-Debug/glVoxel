#pragma once

#include "Engine/Core/Buffer.h"
#include "Engine/Core/VertexArray.h"
#include "Engine/Model.h"
#include "World/Voxel.h"
#include "World/OctreeNode.h"

class World
{
private:
  Model *model;

  VertexArray vao;
  Buffer vbo;
  Buffer ebo;

  Buffer ccc; // Center center chunk

  OctreeNode<Voxel> root;

private:
  void setVertexAttribPointer()
  {
    vao.bind();
    vao.set(3, 1, VertexType::UNSIGNED_INT, false, sizeof(Voxel), (void *)offsetof(Voxel, type), 1);
    vao.set(4, 4, VertexType::FLOAT, false, sizeof(Voxel), (void *)offsetof(Voxel, identity), 1);
    vao.set(5, 4, VertexType::FLOAT, false, sizeof(Voxel), (void *)(offsetof(Voxel, identity) + (sizeof(float) * 4)), 1);
    vao.set(6, 4, VertexType::FLOAT, false, sizeof(Voxel), (void *)(offsetof(Voxel, identity) + (sizeof(float) * 8)), 1);
    vao.set(7, 4, VertexType::FLOAT, false, sizeof(Voxel), (void *)(offsetof(Voxel, identity) + (sizeof(float) * 12)), 1);
  }

public:
  World() : vbo(BufferTarget::ARRAY_BUFFER),
            ebo(BufferTarget::ELEMENT_ARRAY_BUFFER),
            ccc(BufferTarget::ARRAY_BUFFER, VertexDraw::DYNAMIC)
  {
    vao.generate();
    vbo.generate();
    ebo.generate();

    ccc.generate();

    vao.bind();
  }

  void setModel(Model *model)
  {
    ebo.set(model->getIndices());
    vbo.set(model->getVertices());

    vao.set(0, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, position));
    vao.set(1, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    vao.set(2, 2, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));

    this->model = model;
  };

  void addVoxel(Voxel &voxel)
  {
    ccc.set(sizeof(Voxel), 1, (const void *)&voxel);
    setVertexAttribPointer();
  }

  void createGrid()
  {
    const Instance &instance = model->getInstance(model->createInstance());
    ccc.set(sizeof(Instance), 1, (const void *)&instance);

    setVertexAttribPointer();
  }

  void update(std::vector<Voxel> voxels)
  {
    ccc.update(0, voxels);
  }
};