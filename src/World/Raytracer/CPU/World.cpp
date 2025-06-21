#include "World.h"
#include "Debug.h"

#include "Components.h"

using namespace RaytracerCPU;

World::World() { m_Voxels.setHeightMap(&heightMap); }

void World::initialize() {
  m_Texture.generate();
  m_Texture.bind();
  m_Texture.setWidth(m_Camera->viewportWidth);
  m_Texture.setHeight(m_Camera->viewportHeight);
  m_Texture.setFilter(TextureFilter::LINEAR, TextureFilter::LINEAR);
  m_Texture.setTexture(GL_RGBA8);
  m_Texture.unbind();

  heightMap.initialize();
  m_Voxels.initialize(m_Camera);
}

void World::draw() {
  m_Texture.bind();
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void World::update() {
  m_Voxels.update();

  for (CTextureBuffer *textureBuffer : m_Registry->get<CTextureBuffer>()) {
    if (textureBuffer->isDirty()) {
      const std::vector<uint32_t> &buffer = textureBuffer->getBuffer();

      LOG("Texture Updated");
      const glm::ivec2 &dimension = textureBuffer->getDimension();
      m_Texture.update((unsigned char *)buffer.data(), dimension.x,
                       dimension.y);

      textureBuffer->clean();
    }
  }
}

void World::setRegistry(Registry *registry) {
  m_Registry = registry;
  m_Voxels.setRegistry(m_Registry);
}

void World::setCamera(PerspectiveCamera *camera) { m_Camera = camera; }