#include "World.h"
#include "Debug.h"

#include "Components.h"

World::World() { voxels.setHeightMap(&heightMap); }

void World::initialize() {
  buffer.generate();

  heightMap.initialize();
  voxels.initialize(camera->position);
}

void World::draw() {
  buffer.bind();

  /**
   * TODO: Eventually we will get a segment fault here because the vertex buffer
   * has a size limit. The solution is to batch vertices and draw them.
   */
  for (CVoxelBuffer *voxelBuffer : registry->get<CVoxelBuffer>())
  {
    LOG("voxelBuffer->getSize()", voxelBuffer->getSize());
    glDrawArrays(static_cast<GLenum>(drawMode), 0, voxelBuffer->getSize());
  }

  buffer.sync();
}

void World::update() {
  voxels.update(camera->position);

  for (CVoxelBuffer *voxelBuffer : registry->get<CVoxelBuffer>()) {
    if (voxelBuffer->isDirty() || buffer.isDirty()) {
      const std::vector<Vertex> verticies = voxelBuffer->getVertices();
      auto [vao, vbo] = buffer.get();

      vao->bind();
      vbo->set(verticies);
      vao->set(0, 3, VertexType::FLOAT, false, sizeof(Vertex),
               (void *)(offsetof(Vertex, x)));
      vao->set(1, 3, VertexType::FLOAT, false, sizeof(Vertex),
               (void *)(offsetof(Vertex, nx)));
      vao->set(2, 1, VertexType::INT, false, sizeof(Vertex),
               (void *)(offsetof(Vertex, color)));
      vao->set(3, 1, VertexType::INT, false, sizeof(Vertex),
               (void *)(offsetof(Vertex, material)));

      voxelBuffer->clean();
    }
  }
}

void World::setRegistry(Registry *registry) {
  this->registry = registry;
  voxels.setRegistry(registry);
}

void World::setCamera(PerspectiveCamera *camera) { this->camera = camera; }