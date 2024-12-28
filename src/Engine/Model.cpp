#include "Model.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Debug.h"
#include "OBJLoader.h"

void loadOBJ(const std::string &filepath, std::vector<Mesh> &meshes)
{
  OBJLoader loader(filepath, meshes);
}

Model::Model(unsigned int id, const char *filepath) : id(id)
{
  loadOBJ(filepath, meshes);
}

Model::~Model() {}

const unsigned int Model::getID() const
{
  return id;
}

const unsigned int Model::createInstance()
{
  instances.emplace_back();
  return instances.size() - 1;
}

Instance &Model::getInstance(unsigned int id)
{
  return instances.at(id);
}

std::vector<Instance> &Model::getInstances()
{
  return instances;
}

const std::vector<Vertex> Model::getVertices() const
{
  std::vector<Vertex> vertices;

  size_t count = 0;
  for (const auto &mesh : meshes)
    count += mesh.vertices.size();

  vertices.reserve(count);

  for (const auto &mesh : meshes)
    vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());

  return vertices;
}

const std::vector<unsigned int> Model::getIndices() const
{
  std::vector<unsigned int> indices;

  size_t count = 0;

  for (const auto &mesh : meshes)
    count += mesh.indices.size();

  indices.reserve(count);

  for (const auto &mesh : meshes)
    indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());

  return indices;
}