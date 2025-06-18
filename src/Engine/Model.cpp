#include "Model.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Debug.h"

Model::Model(unsigned int id, const char *filepath) : m_Id(id) {
  (void)filepath;
  // loadOBJ(filepath, meshes);
}

Model::~Model() {}

unsigned int Model::getID() { return m_Id; }

unsigned int Model::createInstance() {
  m_Instances.emplace_back();
  return static_cast<unsigned int>(m_Instances
  .size()) - 1;
}

Instance &Model::getInstance(unsigned int id) { return m_Instances.at(id); }

std::vector<Instance> &Model::getInstances() { return m_Instances; }

const std::vector<Vertex> Model::getVertices() const {
  std::vector<Vertex> vertices;

  size_t count = 0;
  for (const auto &mesh : m_Meshes)
    count += mesh.vertices.size();

  vertices.reserve(count);

  for (const auto &mesh : m_Meshes)
    vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());

  return vertices;
}

const std::vector<unsigned int> Model::getIndices() const {
  std::vector<unsigned int> indices;

  size_t count = 0;

  for (const auto &mesh : m_Meshes)
    count += mesh.indices.size();

  indices.reserve(count);

  for (const auto &mesh : m_Meshes)
    indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());

  return indices;
}