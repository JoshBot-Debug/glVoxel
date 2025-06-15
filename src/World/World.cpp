#include "World.h"
#include "Debug.h"

#include "Components.h"

World::World() { voxels.setHeightMap(&heightMap); }

void World::initialize() {
  vao.generate();
  vbo.generate();
  heightMap.initialize();
  voxels.initialize(camera->position);
}

void World::draw() {
  vao.bind();
  for (CVoxelBuffer *voxelBuffer : registry->get<CVoxelBuffer>())
    glDrawArrays(static_cast<GLenum>(drawMode), 0, voxelBuffer->getSize());
}

void World::update() {
  voxels.update(camera->position);

  for (CVoxelBuffer *voxelBuffer : registry->get<CVoxelBuffer>()) {
    if (voxelBuffer->isDirty()) {
      vao.bind();
      vbo.set(voxelBuffer->getVertices());
      vao.set(0, 3, VertexType::FLOAT, false, sizeof(Vertex),
              (void *)(offsetof(Vertex, x)));
      vao.set(1, 3, VertexType::FLOAT, false, sizeof(Vertex),
              (void *)(offsetof(Vertex, nx)));
      vao.set(2, 1, VertexType::INT, false, sizeof(Vertex),
              (void *)(offsetof(Vertex, color)));
      vao.set(3, 1, VertexType::INT, false, sizeof(Vertex),
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