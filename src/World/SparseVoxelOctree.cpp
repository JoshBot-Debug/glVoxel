#include "SparseVoxelOctree.h"
#include <cmath>
#include <iostream>
#include "GreedyMesh.h"
#include <omp.h>

Voxel::SparseVoxelOctree::SparseVoxelOctree() : size(256), maxDepth(8), root(new Node{}) {}

Voxel::SparseVoxelOctree::SparseVoxelOctree(int size)
    : size(size), maxDepth(std::log2(size)), root(new Node{}) {}

Voxel::SparseVoxelOctree::~SparseVoxelOctree()
{
  clear();
  delete root;
  root = nullptr;
}

const int Voxel::SparseVoxelOctree::getSize() const
{
  return size;
}

const int Voxel::SparseVoxelOctree::getMaxDepth() const
{
  return maxDepth;
}

Voxel::SparseVoxelOctree::Node *Voxel::SparseVoxelOctree::getRoot()
{
  return root;
}

void Voxel::SparseVoxelOctree::set(glm::vec3 position, Type voxelType)
{
  set(static_cast<int>(position.x), static_cast<int>(position.y), static_cast<int>(position.z), voxelType);
}

void Voxel::SparseVoxelOctree::set(int x, int y, int z, Type voxelType)
{
  set(root, x, y, z, voxelType, size);
}

Voxel::SparseVoxelOctree::Node *Voxel::SparseVoxelOctree::get(glm::vec3 position)
{
  return get(static_cast<int>(position.x), static_cast<int>(position.y), static_cast<int>(position.z));
}

Voxel::SparseVoxelOctree::Node *Voxel::SparseVoxelOctree::get(int x, int y, int z)
{
  return get(root, x, y, z, size);
}

void Voxel::SparseVoxelOctree::set(Node *node, int x, int y, int z, Type voxelType, int size)
{
  if (size == 1)
  {
    node->voxelType = voxelType;
    return;
  }

  int half = size / 2;

  int index = ((x >= half) << 2) | ((y >= half) << 1) | (z >= half);

  if (!node->children[index])
    node->children[index] = new Node{(uint8_t)(maxDepth - std::log2(half))};

  this->set(node->children[index], x % half, y % half, z % half, voxelType, half);

  if (!node->children[0])
    return;

  Type firstType = node->children[0]->voxelType;

  for (int i = 0; i < 8; i++)
    if (!node->children[i] || node->children[i]->voxelType != firstType || node->children[i]->children[0] != nullptr)
      return;

  for (int i = 0; i < 8; i++)
  {
    delete node->children[i];
    node->children[i] = nullptr;
  }

  node->voxelType = firstType;
}

Voxel::SparseVoxelOctree::Node *Voxel::SparseVoxelOctree::get(Node *node, int x, int y, int z, int size)
{
  if (x < 0 || y < 0 || z < 0 || x >= size || y >= size || z >= size)
    return nullptr;

  if (!node)
    return nullptr;

  if ((uint8_t)node->voxelType)
  {
    if ((uint8_t)lockedType && node->voxelType != lockedType)
      return nullptr;
    else
      return node;
  }

  int half = size / 2;

  int index = ((x >= half) << 2) | ((y >= half) << 1) | (z >= half);

  return get(node->children[index], x % half, y % half, z % half, half);
}

void Voxel::SparseVoxelOctree::clear(Node *node)
{
  if (!node)
    return;

  node->voxelType = Voxel::Type::NONE;
  node->depth = 0;

  for (int i = 0; i < 8; i++)
  {
    if (node->children[i])
    {
      clear(node->children[i]);
      delete node->children[i];
      node->children[i] = nullptr;
    }
  }
}

void Voxel::SparseVoxelOctree::clear()
{
  clear(root);
}

void Voxel::SparseVoxelOctree::greedyMesh(std::vector<Vertex> &vertices)
{
  const int chunkSize = 32;
  const int chunksPerAxis = size / chunkSize;

  // Thread-safe: each thread gets its own local vector
  std::vector<std::vector<Vertex>> tVertices;

  int maxThreads = omp_get_max_threads();
  tVertices.resize(maxThreads);

#pragma omp parallel for collapse(3)
  for (int cz = 0; cz < chunksPerAxis; cz++)
    for (int cy = 0; cy < chunksPerAxis; cy++)
      for (int cx = 0; cx < chunksPerAxis; cx++)
        GreedyMesh::SparseVoxelTree(this, tVertices[omp_get_thread_num()], cx * chunkSize, cy * chunkSize, cz * chunkSize, chunkSize, chunkSize * chunkSize);

  for (const auto &v : tVertices)
    vertices.insert(vertices.end(), v.begin(), v.end());

  // Merge them with vertices here
  std::cout << "Voxels (Million): " << (double)(size * size * size) / 1000000.0 << std::endl;
  std::cout << "Memory (MB): " << (double)getTotalMemoryUsage() / 1000000.0 << std::endl;
}

void Voxel::SparseVoxelOctree::lock(Voxel::Type type)
{
  lockedType = type;
}

void Voxel::SparseVoxelOctree::unlock()
{
  lockedType = Type::NONE;
}

const size_t Voxel::SparseVoxelOctree::getMemoryUsage(Node *node) const
{
  if (!node)
    return 0;

  size_t size = sizeof(SparseVoxelOctree::Node);

  for (int i = 0; i < 8; ++i)
    size += getMemoryUsage(node->children[i]);

  return size;
}

const size_t Voxel::SparseVoxelOctree::getTotalMemoryUsage() const
{
  return sizeof(SparseVoxelOctree) + getMemoryUsage(root);
}
