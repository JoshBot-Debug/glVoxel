#include "SparseVoxelOctree.h"
#include <cmath>
#include <iostream>
#include "World/GreedyMesh.h"
#include <omp.h>

SparseVoxelOctree::SparseVoxelOctree() : size(256), maxDepth(8), root(new Node(0)) {}

SparseVoxelOctree::SparseVoxelOctree(int size)
    : size(size), maxDepth(std::log2(size)), root(new Node(0)) {}

SparseVoxelOctree::~SparseVoxelOctree()
{
  clear();
}

const int SparseVoxelOctree::getSize() const
{
  return size;
}

const int SparseVoxelOctree::getMaxDepth() const
{
  return maxDepth;
}

Node *SparseVoxelOctree::getRoot()
{
  return root;
}

void SparseVoxelOctree::set(glm::vec3 position, Voxel *voxel)
{
  set(static_cast<int>(position.x), static_cast<int>(position.y), static_cast<int>(position.z), voxel);
}

void SparseVoxelOctree::set(int x, int y, int z, Voxel *voxel)
{
  set(root, x, y, z, voxel, size);
}

Node *SparseVoxelOctree::get(glm::vec3 position, int maxDepth, Voxel *filter)
{
  return get(static_cast<int>(position.x), static_cast<int>(position.y), static_cast<int>(position.z), maxDepth, filter);
}

Node *SparseVoxelOctree::get(int x, int y, int z, int maxDepth, Voxel *filter)
{
  return get(root, x, y, z, size, maxDepth, filter);
}

void SparseVoxelOctree::set(Node *node, int x, int y, int z, Voxel *voxel, int size)
{
  if (size == 1)
  {
    node->voxel = voxel;
    for (const Voxel &uVoxel : uniqueVoxels)
      if (uVoxel == *voxel)
        return;

    uniqueVoxels.push_back({voxel->color, voxel->material});
    return;
  }

  int half = size / 2;

  int index = ((x >= half) << 2) | ((y >= half) << 1) | (z >= half);

  if (!node->children[index])
    node->children[index] = new Node((uint8_t)(maxDepth - std::log2(half)));

  this->set(node->children[index], x % half, y % half, z % half, voxel, half);

  if (!node->children[0] || !node->children[0]->voxel)
    return;

  /**
   * If all 8 children exist and are of the same voxel type.
   * Delete all 8 children and set the parent voxel as their type.
   */
  Voxel firstVoxel = *node->children[0]->voxel;

  for (int i = 0; i < 8; i++)
    if (!node->children[i] || !node->children[i]->voxel || *node->children[i]->voxel != firstVoxel)
      return;

  for (int i = 0; i < 8; i++)
  {
    delete node->children[i];
    node->children[i] = nullptr;
  }

  node->voxel = new Voxel(firstVoxel.color, firstVoxel.material);
}

inline int floorDiv(int a, int b)
{
  return (a >= 0) ? (a / b) : ((a - b + 1) / b);
}

inline int mod(int a, int b)
{
  int r = a % b;
  return (r < 0) ? r + b : r;
}

Node *SparseVoxelOctree::get(Node *node, int x, int y, int z, int size, int maxDepth, Voxel *filter)
{
  if (!node)
    return nullptr;

  // out of bounds, need to check the neighbour svo chunk
  if (x < 0 || y < 0 || z < 0 || x >= size || y >= size || z >= size)
  {

    // std::cout << std::dec;
    // if (y < 0 || y >= size)
    //   return nullptr;

    // if (chunkCoord.x != 0)
    //   return nullptr;

    // This is where I need to get the voxel from the neighbour SVO if one exists

    const glm::ivec2 nPos = {floorDiv(x, this->size) + chunkCoord.x, floorDiv(z, this->size) + chunkCoord.y};

    // std::cout << "Find Voxel: " << (int)x << " " << (int)y << " " << (int)z << std::endl;
    // std::cout << "Neighbour Coord: " << (int)nPos.x << " " << (int)nPos.y << std::endl;
    // std::cout << "My Coord: " << (int)chunkCoord.x << " " << (int)chunkCoord.y << std::endl;

    auto it = neighbours.find(nPos);

    if (it == neighbours.end() || it->second == nullptr)
      return nullptr;

    SparseVoxelOctree *neighbour = it->second;

    const glm::vec3 localCoord = {
        mod(x, this->size),
        y,
        mod(z, this->size)};

    // std::cout << "Neighbour Voxel: " << (int)localCoord.x << " " << (int)localCoord.y << " " << (int)localCoord.z << std::endl;
    
    return neighbour->get(localCoord, -1, filter);
  }

  if (node->voxel)
  {
    if (filter && *filter != *node->voxel)
      return nullptr;
    return node;
  }

  int half = size / 2;

  int index = ((x >= half) << 2) | ((y >= half) << 1) | (z >= half);

  return get(node->children[index], x % half, y % half, z % half, half, maxDepth, filter);
}

void SparseVoxelOctree::clear(Node *node)
{
  if (!node)
    return;

  delete node;
  node = nullptr;
}

void SparseVoxelOctree::clear()
{
  if (!root)
    return;

  root->clear();
}

void SparseVoxelOctree::greedyMesh(std::vector<Vertex> &vertices, Voxel *filter)
{
  const int chunkSize = 32;
  const int chunksPerAxis = size / chunkSize;

  for (int cz = 0; cz < chunksPerAxis; cz++)
    for (int cy = 0; cy < chunksPerAxis; cy++)
      for (int cx = 0; cx < chunksPerAxis; cx++)
        GreedyMesh::Octree(this, vertices, cx * chunkSize, cy * chunkSize, cz * chunkSize, chunkSize, chunkSize * chunkSize, filter);
}

const size_t SparseVoxelOctree::getMemoryUsage(Node *node) const
{
  if (!node)
    return 0;

  size_t size = sizeof(Node);

  if (node->voxel)
    size += sizeof(Voxel);

  for (int i = 0; i < 8; ++i)
    size += getMemoryUsage(node->children[i]);

  return size;
}

const size_t SparseVoxelOctree::getTotalMemoryUsage() const
{
  return sizeof(SparseVoxelOctree) + getMemoryUsage(root);
}

std::vector<Voxel> SparseVoxelOctree::getUniqueVoxels()
{
  return uniqueVoxels;
}

void SparseVoxelOctree::setNeighbours(const glm::ivec2 &chunkPosition, std::unordered_map<glm::ivec2, SparseVoxelOctree, IVec2Hash, IVec2Equal> &chunks)
{
  chunkCoord = chunkPosition;

  static const std::vector<glm::ivec2> directions = {
      {0, -1}, // N
      {1, -1}, // NE
      {1, 0},  // E
      {1, 1},  // SE
      {0, 1},  // S
      {-1, 1}, // SW
      {-1, 0}, // W
      {-1, -1} // NW
  };

  neighbours.clear();
  neighbours.reserve(directions.size());

  for (const auto &dir : directions)
  {
    glm::ivec2 neighborPosition = chunkPosition + dir;
    auto it = chunks.find(neighborPosition);
    if (it != chunks.end())
      neighbours[neighborPosition] = &it->second;
    else
      neighbours[neighborPosition] = nullptr;
  }
}
