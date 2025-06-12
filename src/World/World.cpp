#include "World.h"
#include "Debug.h"

void World::initialize()
{
  vao.generate();
  vbo.generate();
  heightMap.initialize();
  voxels.initialize({0, 0, 0}, &heightMap);
}

void World::draw()
{
  vao.bind();
  glDrawArrays(static_cast<GLenum>(drawMode), 0, voxels.vertices.size());
}

void World::update()
{
  voxels.update(camera->position);
}

void World::setBuffer()
{
  vao.bind();
  vbo.set(voxels.vertices);
  vao.set(0, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)(offsetof(Vertex, x)));
  vao.set(1, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)(offsetof(Vertex, nx)));
  vao.set(2, 1, VertexType::INT, false, sizeof(Vertex), (void *)(offsetof(Vertex, color)));
  vao.set(3, 1, VertexType::INT, false, sizeof(Vertex), (void *)(offsetof(Vertex, material)));
}

void World::setCamera(PerspectiveCamera *camera)
{
  this->camera = camera;
}