#include "Model.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Debug.h"
#include "OBJLoader.h"

void printMatrix(const glm::mat4 &matrix)
{
  for (int i = 0; i < 4; ++i)
  {
    for (int j = 0; j < 4; ++j)
    {
      std::cout << matrix[i][j] << " ";
    }
    std::cout << std::endl;
  }
}

glm::mat4 toGLMMat4(const aiMatrix4x4 &aiMat)
{
  glm::mat4 glmMat;
  glmMat[0][0] = aiMat.a1;
  glmMat[0][1] = aiMat.a2;
  glmMat[0][2] = aiMat.a3;
  glmMat[0][3] = aiMat.a4;
  glmMat[1][0] = aiMat.b1;
  glmMat[1][1] = aiMat.b2;
  glmMat[1][2] = aiMat.b3;
  glmMat[1][3] = aiMat.b4;
  glmMat[2][0] = aiMat.c1;
  glmMat[2][1] = aiMat.c2;
  glmMat[2][2] = aiMat.c3;
  glmMat[2][3] = aiMat.c4;
  glmMat[3][0] = aiMat.d1;
  glmMat[3][1] = aiMat.d2;
  glmMat[3][2] = aiMat.d3;
  glmMat[3][3] = aiMat.d4;
  return glmMat;
}

glm::mat4 getGlobalTransform(aiNode *node, const aiScene *scene)
{
  glm::mat4 transform = glm::mat4(1.0f);

  while (node)
  {
    printMatrix(toGLMMat4(node->mTransformation));
    transform = toGLMMat4(node->mTransformation) * transform;
    node = node->mParent;
  }

  return transform;
}

void processMesh(Mesh &m, aiMesh *mesh, const glm::mat4 &globalTransform, const aiScene *scene)
{
  m.vertices.resize(mesh->mNumVertices);

  glm::mat4 normalMatrix = glm::transpose(glm::inverse(globalTransform));

  for (unsigned int i = 0; i < mesh->mNumVertices; i++)
  {
    Vertex &v = m.vertices[i];

    glm::vec4 position = globalTransform * glm::vec4(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
    v.position.x = position.x;
    v.position.y = position.y;
    v.position.z = position.z;

    if (mesh->HasNormals())
    {
      glm::vec4 normal = normalMatrix * glm::vec4(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z, 0.0f);
      v.normal.x = normal.x;
      v.normal.y = normal.y;
      v.normal.z = normal.z;
    }

    if (mesh->HasTextureCoords(0))
    {
      v.texCoord.x = mesh->mTextureCoords[0][i].x;
      v.texCoord.y = mesh->mTextureCoords[0][i].y;
    }

    if (mesh->HasTangentsAndBitangents())
    {

      glm::vec4 tangent = normalMatrix * glm::vec4(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z, 0.0f);
      glm::vec4 bitangent = normalMatrix * glm::vec4(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z, 0.0f);

      v.tangent.x = tangent.x;
      v.tangent.y = tangent.y;
      v.tangent.z = tangent.z;

      v.bitangent.x = bitangent.x;
      v.bitangent.y = bitangent.y;
      v.bitangent.z = bitangent.z;
    }

    if (mesh->HasVertexColors(0))
    {
      v.color.x = mesh->mColors[0][i].r;
      v.color.y = mesh->mColors[0][i].g;
      v.color.z = mesh->mColors[0][i].b;
    }
  }

  for (unsigned int i = 0; i < mesh->mNumFaces; i++)
  {
    aiFace &face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++)
      m.indices.push_back(face.mIndices[j]);
  }
};

void processNode(std::vector<Mesh> &meshes, aiNode *node, const aiScene *scene)
{
  const glm::mat4 &globalTransform = getGlobalTransform(node, scene);

  std::cout << "Global Transform for Node: " << node->mName.C_Str() << std::endl;
  printMatrix(globalTransform);

  unsigned int start = meshes.size();
  meshes.resize(start + scene->mNumMeshes);

  for (unsigned int i = 0; i < node->mNumMeshes; i++)
    processMesh(meshes[start + i], scene->mMeshes[node->mMeshes[i]], globalTransform, scene);

  for (unsigned int i = 0; i < node->mNumChildren; i++)
    processNode(meshes, node->mChildren[i], scene);
}

void loadModel(const char *path, std::vector<Mesh> &meshes)
{
  Assimp::Importer importer;

  const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_FlipWindingOrder | aiProcess_GenSmoothNormals | aiProcess_OptimizeMeshes);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
  {
    LOG_BREAK_BEFORE;
    LOG("Failed to load model:", importer.GetErrorString());
    LOG_BREAK_AFTER;
    return;
  }

  processNode(meshes, scene->mRootNode, scene);
}

void loadOBJ(const std::string &filepath, std::vector<Mesh> &meshes)
{
  OBJLoader loader(filepath, meshes);
}

Model::Model(unsigned int id, const char *filepath) : id(id)
{
  loadOBJ(filepath, meshes);
  // loadModel(filepath, meshes);
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