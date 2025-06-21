#include "SparseVoxelOctree.h"

#include <cmath>
#include <iostream>

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
    : m_Size(256), m_Depth(8), m_Root(new Node(8)) {}

SparseVoxelOctree::SparseVoxelOctree(int size)
    : m_Size(size), m_Depth(static_cast<uint8_t>(std::log2(size))),
      m_Root(new Node(static_cast<uint8_t>(std::log2(size)))) {}

SparseVoxelOctree::~SparseVoxelOctree() {
  clear();
  delete m_Root;
  m_Root = nullptr;
}

int SparseVoxelOctree::getSize() { return m_Size; }

int SparseVoxelOctree::getDepth() { return m_Depth; }

Node *SparseVoxelOctree::getRoot() { return m_Root; }

void SparseVoxelOctree::set(uint64_t (&mask)[], Voxel *voxel) {
  for (int z = 0; z < m_Size; z += 64)
    for (int x = 0; x < m_Size; x += 64)
      for (int y = 0; y < m_Size; y += 64)
        set(mask, x, y, z, voxel, 64);
}

void SparseVoxelOctree::set(uint64_t (&mask)[], int x, int y, int z,
                                 Voxel *voxel, int size) {
  bool isFullBlock = true;

  for (int dz = 0; dz < size && isFullBlock; ++dz)
    for (int dx = 0; dx < size && isFullBlock; ++dx)
      for (int dy = 0; dy < size && isFullBlock; ++dy) {
        int ix = x + dx;
        int iy = y + dy;
        int iz = z + dz;

        int index = ix + m_Size * (iz + m_Size * iy);
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
    int index = x + m_Size * (z + m_Size * y);
    if (mask[index / 64] & (1UL << (index % 64)))
      set(x, y, z, voxel, 1);
    return;
  }

  int half = size / 2;

  for (int dz = 0; dz < size; dz += half)
    for (int dx = 0; dx < size; dx += half)
      for (int dy = 0; dy < size; dy += half)
        set(mask, x + dx, y + dy, z + dz, voxel, half);
}

void SparseVoxelOctree::set(glm::vec3 position, Voxel *voxel, int leafSize) {
  set(static_cast<int>(position.x), static_cast<int>(position.y),
      static_cast<int>(position.z), voxel, leafSize);
}

void SparseVoxelOctree::set(int x, int y, int z, Voxel *voxel, int leafSize) {
  set(m_Root, x, y, z, voxel, leafSize, m_Size);
}

Node *SparseVoxelOctree::get(glm::vec3 position, Voxel *filter) {
  return get(static_cast<int>(position.x), static_cast<int>(position.y),
             static_cast<int>(position.z), filter);
}

Node *SparseVoxelOctree::get(int x, int y, int z, Voxel *filter) {
  return get(m_Root, x, y, z, m_Size, filter);
}

void SparseVoxelOctree::set(Node *node, int x, int y, int z, Voxel *voxel,
                            int leafSize, int size) {
  if (size == leafSize) {
    node->voxel = voxel;
    return;
  }

  int half = size / 2;

  int index = ((x >= half) << 2) | ((y >= half) << 1) | (z >= half);

  if (!node->children[index])
    node->children[index] = new Node(static_cast<uint8_t>(std::log2(half)));

  this->set(node->children[index], x % half, y % half, z % half, voxel,
            leafSize, half);

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
                             Voxel *filter) {
  if (!node)
    return nullptr;

  if (x < 0 || y < 0 || z < 0 || x >= size || y >= size || z >= size) {
    const glm::ivec3 np = {floorDiv(x, m_Size) + m_ChunkCoord.x,
                           floorDiv(y, m_Size) + m_ChunkCoord.y,
                           floorDiv(z, m_Size) + m_ChunkCoord.z};

    auto it = m_Neighbours.find(np);

    if (it == m_Neighbours.end() || it->second == nullptr)
      return nullptr;

    return it->second->get({mod(x, m_Size), mod(y, m_Size), mod(z, m_Size)},
                           filter);
  }

  if (node->voxel) {
    if (filter && *filter != *node->voxel)
      return nullptr;
    return node;
  }

  int half = size / 2;

  int index = ((x >= half) << 2) | ((y >= half) << 1) | (z >= half);

  return get(node->children[index], x % half, y % half, z % half, half, filter);
}

void SparseVoxelOctree::clear() {
  if (!m_Root)
    return;

  m_Root->clear();
}

size_t SparseVoxelOctree::getMemoryUsage(Node *node) {
  if (!node)
    return 0;

  size_t size = sizeof(Node);

  if (node->voxel)
    size += sizeof(Voxel);

  for (int i = 0; i < 8; ++i)
    size += getMemoryUsage(node->children[i]);

  return size;
}

void SparseVoxelOctree::setNeighbours(
    const glm::ivec3 &chunkCoord,
    const std::unordered_map<glm::ivec3, SparseVoxelOctree *> &chunks) {
  m_ChunkCoord = chunkCoord;

  m_Neighbours.clear();
  m_Neighbours.reserve(NEIGHBOUR_DIRECTIONS.size());

  for (const glm::ivec3 &dir : NEIGHBOUR_DIRECTIONS) {
    glm::ivec3 np = chunkCoord + dir;
    if (chunks.contains(np))
      m_Neighbours[np] = chunks.at(np);
    else
      m_Neighbours[np] = nullptr;
  }
}

size_t SparseVoxelOctree::getTotalMemoryUsage() {
  return sizeof(SparseVoxelOctree) + getMemoryUsage(m_Root);
}