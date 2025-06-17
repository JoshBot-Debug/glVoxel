#include <cmath>
#include <iostream>

#include "SparseVoxelOctree.h"
#include "Voxel/GreedyMesh32.h"
#include "Voxel/GreedyMesh64.h"

static const std::vector<glm::ivec3> NEIGHBOUR_DIRECTIONS =
    {               // Cardinal directions (6)
        {1, 0, 0},  // +X (East)
        {-1, 0, 0}, // -X (West)
        {0, 1, 0},  // +Y (Up)
        {0, -1, 0}, // -Y (Down)
        {0, 0, 1},  // +Z (South)
        {0, 0, -1}, // -Z (North)

        // Face diagonals (12)
        {1, 1, 0},
        {1, -1, 0},
        {-1, 1, 0},
        {-1, -1, 0},
        {1, 0, 1},
        {1, 0, -1},
        {-1, 0, 1},
        {-1, 0, -1},
        {0, 1, 1},
        {0, 1, -1},
        {0, -1, 1},
        {0, -1, -1},

        // Corner diagonals (8)
        {1, 1, 1},
        {1, 1, -1},
        {1, -1, 1},
        {1, -1, -1},
        {-1, 1, 1},
        {-1, 1, -1},
        {-1, -1, 1},
        {-1, -1, -1}};

SparseVoxelOctree::SparseVoxelOctree()
    : size(256), depth(8), root(new Node(8)) {}

SparseVoxelOctree::SparseVoxelOctree(int size)
    : size(size), depth(std::log2(size)),
      root(new Node(static_cast<uint8_t>(std::log2(size)))) {}

SparseVoxelOctree::~SparseVoxelOctree() {
  clear();
  delete root;
  root = nullptr;
}

const int SparseVoxelOctree::getSize() const { return size; }

const int SparseVoxelOctree::getDepth() const { return depth; }

Node *SparseVoxelOctree::getRoot() { return root; }

void SparseVoxelOctree::setBlock(uint64_t (&mask)[], Voxel *voxel) {
  for (int z = 0; z < size; z += 64)
    for (int x = 0; x < size; x += 64)
      for (int y = 0; y < size; y += 64)
        setBlock(mask, x, y, z, voxel, 64);
}

void SparseVoxelOctree::setBlock(uint64_t (&mask)[], int x, int y, int z,
                                 Voxel *voxel, int size) {
  bool isFullBlock = true;

  for (int dz = 0; dz < size && isFullBlock; ++dz)
    for (int dx = 0; dx < size && isFullBlock; ++dx)
      for (int dy = 0; dy < size && isFullBlock; ++dy) {
        int ix = x + dx;
        int iy = y + dy;
        int iz = z + dz;

        int index = ix + this->size * (iz + this->size * iy);
        if (!(mask[index / 64] & (1UL << (index % 64)))) {
          isFullBlock = false;
          break;
        }
      }

  if (isFullBlock) {
    set(x, y, z, voxel, size);
    return;
  }

  if (size == 1) {
    int index = x + this->size * (z + this->size * y);
    if (mask[index / 64] & (1UL << (index % 64)))
      set(x, y, z, voxel, 1);
    return;
  }

  int half = size / 2;

  for (int dz = 0; dz < size; dz += half)
    for (int dx = 0; dx < size; dx += half)
      for (int dy = 0; dy < size; dy += half)
        setBlock(mask, x + dx, y + dy, z + dz, voxel, half);
}

void SparseVoxelOctree::set(glm::vec3 position, Voxel *voxel, int maxSize) {
  set(static_cast<int>(position.x), static_cast<int>(position.y),
      static_cast<int>(position.z), voxel, maxSize);
}

void SparseVoxelOctree::set(int x, int y, int z, Voxel *voxel, int maxSize) {
  set(root, x, y, z, voxel, size, maxSize);
}

Node *SparseVoxelOctree::get(glm::vec3 position, uint8_t maxDepth,
                             Voxel *filter) {
  return get(static_cast<int>(position.x), static_cast<int>(position.y),
             static_cast<int>(position.z), maxDepth, filter);
}

Node *SparseVoxelOctree::get(int x, int y, int z, uint8_t maxDepth,
                             Voxel *filter) {
  return get(root, x, y, z, size, maxDepth, filter);
}

void SparseVoxelOctree::set(Node *node, int x, int y, int z, Voxel *voxel,
                            int size, int maxSize) {
  if (size == maxSize) {
    node->voxel = voxel;

    for (const Voxel *v : uniqueVoxels)
      if (v == voxel)
        return;

    uniqueVoxels.push_back(voxel);
    return;
  }

  int half = size / 2;

  int index = ((x >= half) << 2) | ((y >= half) << 1) | (z >= half);

  if (!node->children[index])
    node->children[index] = new Node(static_cast<uint8_t>(std::log2(half)));

  this->set(node->children[index], x % half, y % half, z % half, voxel, half,
            maxSize);

  if (!node->children[0] || !node->children[0]->voxel)
    return;

  /**
   * If all 8 children exist and are of the same voxel type.
   * Delete all 8 children and set the parent voxel as their type.
   */
  Voxel *firstVoxel = node->children[0]->voxel;

  for (int i = 0; i < 8; i++)
    if (!node->children[i] || !node->children[i]->voxel ||
        node->children[i]->voxel != firstVoxel)
      return;

  for (int i = 0; i < 8; i++) {
    delete node->children[i];
    node->children[i] = nullptr;
  }

  node->voxel = firstVoxel;
}

int SparseVoxelOctree::floorDiv(int a, int b) {
  return (a >= 0) ? (a / b) : ((a - b + 1) / b);
}

int SparseVoxelOctree::mod(int a, int b) {
  int r = a % b;
  return (r < 0) ? r + b : r;
}

Node *SparseVoxelOctree::get(Node *node, int x, int y, int z, int size,
                             uint8_t maxDepth, Voxel *filter) {
  if (!node)
    return nullptr;

  if (x < 0 || y < 0 || z < 0 || x >= size || y >= size || z >= size) {
    const glm::ivec3 np = {floorDiv(x, this->size) + globalPosition.x,
                           floorDiv(y, this->size) + globalPosition.y,
                           floorDiv(z, this->size) + globalPosition.z};

    auto it = neighbours.find(np);

    if (it == neighbours.end() || it->second == nullptr)
      return nullptr;

    return it->second->get(
        {mod(x, this->size), mod(y, this->size), mod(z, this->size)}, -1,
        filter);
  }

  if (node->voxel) {
    if (filter && *filter != *node->voxel)
      return nullptr;
    return node;
  }

  int half = size / 2;

  int index = ((x >= half) << 2) | ((y >= half) << 1) | (z >= half);

  return get(node->children[index], x % half, y % half, z % half, half,
             maxDepth, filter);
}

void SparseVoxelOctree::clear(Node *node) {
  if (!node)
    return;

  delete node;
  node = nullptr;
}

void SparseVoxelOctree::clear() {
  if (!root)
    return;

  root->clear();
}

void SparseVoxelOctree::greedyMesh(std::vector<Vertex> &vertices,
                                   Voxel *filter) {
  const int chunkSize = GreedyMesh64::CHUNK_SIZE;
  const int chunksPerAxis = size / chunkSize;

  for (int cz = 0; cz < chunksPerAxis; cz++)
    for (int cy = 0; cy < chunksPerAxis; cy++)
      for (int cx = 0; cx < chunksPerAxis; cx++)
        GreedyMesh64::Octree(this, vertices, cx * chunkSize, cy * chunkSize,
                             cz * chunkSize, 0, filter);
}

const size_t SparseVoxelOctree::getMemoryUsage(Node *node) const {
  if (!node)
    return 0;

  size_t size = sizeof(Node);

  if (node->voxel)
    size += sizeof(Voxel);

  for (int i = 0; i < 8; ++i)
    size += getMemoryUsage(node->children[i]);

  return size;
}

const size_t SparseVoxelOctree::getTotalMemoryUsage() const {
  return sizeof(SparseVoxelOctree) + getMemoryUsage(root);
}

const std::vector<Voxel *> &SparseVoxelOctree::getUniqueVoxels() const {
  return uniqueVoxels;
}

void SparseVoxelOctree::setNeighbours(
    const glm::ivec3 &globalPosition,
    std::unordered_map<glm::ivec3, SparseVoxelOctree *> &chunks) {
  this->globalPosition = globalPosition;

  this->neighbours.clear();
  this->neighbours.reserve(NEIGHBOUR_DIRECTIONS.size());

  for (const glm::ivec3 &dir : NEIGHBOUR_DIRECTIONS) {
    glm::ivec3 np = globalPosition + dir;
    if (chunks.contains(np))
      this->neighbours[np] = chunks.at(np);
    else
      this->neighbours[np] = nullptr;
  }
}