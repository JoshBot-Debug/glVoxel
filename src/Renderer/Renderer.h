#pragma once

#include "Engine/Core/Buffer.h"
#include "Engine/Core/VertexArray.h"
#include "Engine/Model.h"

class Renderer
{
private:
  Model *model;

  VertexArray vao;
  Buffer vbo;
  Buffer ebo;

  Buffer ccc; // Center center chunk

public:
  Renderer() : vbo(BufferTarget::ARRAY_BUFFER),
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
    vao.set(3, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, tangent));
    vao.set(4, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, bitangent));
    vao.set(5, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, color));

    this->model = model;
  };

  void addInstance(const unsigned int id)
  {
    const Instance &instance = model->getInstance(id);
    ccc.set(sizeof(Instance), 1, (const void *)&instance);

    vao.bind();
    vao.set(6, 4, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)offsetof(InstanceBuffer, model), 1);
    vao.set(7, 4, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)(offsetof(InstanceBuffer, model) + (sizeof(float) * 4)), 1);
    vao.set(8, 4, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)(offsetof(InstanceBuffer, model) + (sizeof(float) * 8)), 1);
    vao.set(9, 4, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)(offsetof(InstanceBuffer, model) + (sizeof(float) * 12)), 1);
    vao.set(10, 3, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)offsetof(InstanceBuffer, normalMatrix), 1);
    vao.set(11, 3, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)(offsetof(InstanceBuffer, normalMatrix) + (sizeof(float) * 3)), 1);
    vao.set(12, 3, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)(offsetof(InstanceBuffer, normalMatrix) + (sizeof(float) * 6)), 1);
    vao.set(13, 3, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)offsetof(InstanceBuffer, color), 1);
  }

  void update(std::vector<Instance> instances)
  {
    std::vector<InstanceBuffer> buffers;

    for (auto &instance : instances)
      buffers.push_back(instance.update());

    ccc.update(0, buffers);
  }
};