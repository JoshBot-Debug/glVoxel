#include "World.h"
#include "Debug.h"

#include "Components.h"

using namespace RaytracerCPU;

World::World() { m_Voxels.setHeightMap(&heightMap); }

void World::initialize() {
  // m_Buffer.generate();

  texture.generate();
  texture.bind();
  texture.setWidth(m_Camera->viewportWidth);
  texture.setHeight(m_Camera->viewportHeight);
  texture.setFilter(TextureFilter::NEAREST, TextureFilter::NEAREST);
  texture.setTexture(GL_RGBA8);
  texture.unbind();

  heightMap.initialize();
  m_Voxels.initialize(m_Camera);
}

void World::draw() {
  // m_Buffer.bind();
  texture.bind();

  glDrawArrays(GL_TRIANGLES, 0, 6);
  // for (CVoxelBuffer *voxelBuffer : m_Registry->get<CVoxelBuffer>())
  //   glDrawArrays(static_cast<GLenum>(drawMode), 0, voxelBuffer->getSize());

  // m_Buffer.sync();
}

void World::update() {
  m_Voxels.update();

  texture.resize(m_Camera->viewportWidth, m_Camera->viewportHeight);

  for (CVoxelBuffer *voxelBuffer : m_Registry->get<CVoxelBuffer>()) {
    if (voxelBuffer->isDirty()) {
      const std::vector<uint32_t> &buffer = voxelBuffer->getBuffer();

      texture.update((unsigned char *)buffer.data());

      voxelBuffer->clean();
    }
  }
}

void World::setRegistry(Registry *registry) {
  m_Registry = registry;
  m_Voxels.setRegistry(m_Registry);
}

void World::setCamera(PerspectiveCamera *camera) { m_Camera = camera; }