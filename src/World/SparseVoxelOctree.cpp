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

void Voxel::SparseVoxelOctree::set(glm::vec3 position, int color)
{
  set(static_cast<int>(position.x), static_cast<int>(position.y), static_cast<int>(position.z), color);
}

void Voxel::SparseVoxelOctree::set(int x, int y, int z, int color)
{
  set(root, x, y, z, color, size);
}

Voxel::SparseVoxelOctree::Node *Voxel::SparseVoxelOctree::get(glm::vec3 position)
{
  return get(static_cast<int>(position.x), static_cast<int>(position.y), static_cast<int>(position.z));
}

Voxel::SparseVoxelOctree::Node *Voxel::SparseVoxelOctree::get(int x, int y, int z)
{
  return get(root, x, y, z, size);
}

void Voxel::SparseVoxelOctree::set(Node *node, int x, int y, int z, int color, int size)
{
  if (size == 1)
  {
    node->color = color;
    return;
  }

  int half = size / 2;

  int index = ((x >= half) << 2) | ((y >= half) << 1) | (z >= half);

  if (!node->children[index])
    node->children[index] = new Node{(uint8_t)(maxDepth - std::log2(half))};

  this->set(node->children[index], x % half, y % half, z % half, color, half);

  if (!node->children[0])
    return;

  Node firstNode = *node->children[0];

  for (int i = 0; i < 8; i++)
    if (!node->children[i] || node->children[i]->color != firstNode.color || node->children[i]->children[0] != nullptr)
      return;

  for (int i = 0; i < 8; i++)
  {
    delete node->children[i];
    node->children[i] = nullptr;
  }

  node->color = firstNode.color;
  node->material = firstNode.material;
}

Voxel::SparseVoxelOctree::Node *Voxel::SparseVoxelOctree::get(Node *node, int x, int y, int z, int size)
{
  if (!node || x < 0 || y < 0 || z < 0 || x >= size || y >= size || z >= size)
    return nullptr;

  if ((uint8_t)node->color)
    if ((uint8_t)lockedColor && node->color != lockedColor)
      return nullptr;
    else
      return node;

  int half = size / 2;

  int index = ((x >= half) << 2) | ((y >= half) << 1) | (z >= half);

  return get(node->children[index], x % half, y % half, z % half, half);
}

void Voxel::SparseVoxelOctree::clear(Node *node)
{
  if (!node)
    return;

  node->depth = 0;
  node->color = 0;
  node->material = 0;

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

  std::vector<std::vector<Vertex>> tVertices;

  int maxThreads = omp_get_max_threads();
  tVertices.resize(maxThreads);

#pragma omp parallel for collapse(3)
  for (int cz = 0; cz < chunksPerAxis; cz++)
    for (int cy = 0; cy < chunksPerAxis; cy++)
      for (int cx = 0; cx < chunksPerAxis; cx++)
        GreedyMesh::SparseVoxelTree(this, tVertices[omp_get_thread_num()], cx * chunkSize, cy * chunkSize, cz * chunkSize, chunkSize, chunkSize * chunkSize);

  for (auto &v : tVertices)
    vertices.insert(vertices.end(),
                    std::make_move_iterator(v.begin()),
                    std::make_move_iterator(v.end()));

  std::cout << "Voxels (Million): " << (double)(size * size * size) / 1000000.0 << std::endl;
  std::cout << "Memory (MB): " << (double)getTotalMemoryUsage() / 1000000.0 << std::endl;
}

void Voxel::SparseVoxelOctree::lock(int color)
{
  lockedColor = color;
}

void Voxel::SparseVoxelOctree::unlock()
{
  lockedColor = 0;
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
