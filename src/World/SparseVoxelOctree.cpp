#include "SparseVoxelOctree.h"
#include <cmath>
#include <iostream>
#include "GreedyMesh.h"

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

void Voxel::SparseVoxelOctree::set(glm::vec3 position, Type type)
{
  set(static_cast<int>(position.x), static_cast<int>(position.y), static_cast<int>(position.z), type);
}

void Voxel::SparseVoxelOctree::set(int x, int y, int z, Type type)
{
  set(root, x, y, z, type, size);
}

Voxel::SparseVoxelOctree::Node *Voxel::SparseVoxelOctree::get(glm::vec3 position)
{
  return get(static_cast<int>(position.x), static_cast<int>(position.y), static_cast<int>(position.z));
}

Voxel::SparseVoxelOctree::Node *Voxel::SparseVoxelOctree::get(int x, int y, int z)
{
  return get(root, x, y, z, size);
}

void Voxel::SparseVoxelOctree::set(Node *node, int x, int y, int z, Type type, int size)
{
  if (size == 1)
  {
    node->type = type;
    return;
  }

  int half = size / 2;
  int index = ((x >= half) << 2) | ((y >= half) << 1) | (z >= half);

  if (!node->children[index])
    node->children[index] = new Node{(uint8_t)(maxDepth - std::log2(half))};

  int cx = x % half;
  int cy = y % half;
  int cz = z % half;

  this->set(node->children[index], cx, cy, cz, type, half);

  if (!node->children[0])
    return;

  Type firstType = node->children[0]->type;

  for (int i = 0; i < 8; i++)
    if (!node->children[i] || node->children[i]->type != firstType || node->children[i]->children[0] != nullptr)
      return;

  for (int i = 0; i < 8; i++)
  {
    delete node->children[i];
    node->children[i] = nullptr;
  }

  node->type = firstType;
}

Voxel::SparseVoxelOctree::Node *Voxel::SparseVoxelOctree::get(Node *node, int x, int y, int z, int size)
{
  if (x < 0 || y < 0 || z < 0 || x >= size || y >= size || z >= size)
    return nullptr;

  if (!node)
    return nullptr;

  if ((uint8_t)node->type)
    return node;

  int half = size / 2;
  int index = ((x >= half) << 2) | ((y >= half) << 1) | (z >= half);

  int cx = x % half;
  int cy = y % half;
  int cz = z % half;

  return get(node->children[index], cx, cy, cz, half);
}

void Voxel::SparseVoxelOctree::clear(Node *node)
{
  if (!node)
    return;

  node->type = Voxel::Type::NONE;
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
  vertices.clear();

  const int chunkSize = 32;
  const int chunksPerAxis = size / chunkSize;

  for (int cz = 0; cz < chunksPerAxis; cz++)
    for (int cy = 0; cy < chunksPerAxis; cy++)
      for (int cx = 0; cx < chunksPerAxis; cx++)
      {
        int originX = cx * chunkSize;
        int originY = cy * chunkSize;
        int originZ = cz * chunkSize;

        GreedyMesh::SparseVoxelTree(this, vertices, originX, originY, originZ, chunkSize, chunkSize * chunkSize);
      }

  std::cout << "Voxels (Million): " << (double)(size * size * size) / 1000000.0 << std::endl;
  std::cout << "Memory (MB): " << (double)getTotalMemoryUsage() / 1000000.0 << std::endl;
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
