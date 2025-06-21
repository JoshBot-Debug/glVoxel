#include "World.h"
#include "Debug.h"

#include "Components.h"

using namespace Raster;

World::World() { m_Voxels.setHeightMap(&heightMap); }

void World::initialize() {
  m_Buffer.generate();

  heightMap.initialize();
  m_Voxels.initialize(m_Camera->position);
}

void World::draw() {
  m_Buffer.bind();

  /**
   * TODO: Eventually we will get a segment fault here because the vertex buffer
   * has a size limit. The solution is to batch vertices and draw them.
   * Anyways I'd want to create indices or something so just ignoring this for now
   */
  for (CVoxelBuffer *voxelBuffer : m_Registry->get<CVoxelBuffer>())
    glDrawArrays(static_cast<GLenum>(drawMode), 0, voxelBuffer->getSize());

  m_Buffer.sync();
}

void World::update() {
  m_Voxels.update(m_Camera->position);

  for (CVoxelBuffer *voxelBuffer : m_Registry->get<CVoxelBuffer>()) {
    if (voxelBuffer->isDirty() || m_Buffer.isDirty()) {
      const std::vector<Vertex> verticies = voxelBuffer->getVertices();
      auto [vao, vbo] = m_Buffer.get();

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
  m_Registry = registry;
  m_Voxels.setRegistry(m_Registry);
}

void World::setCamera(PerspectiveCamera *camera) { m_Camera = camera; }