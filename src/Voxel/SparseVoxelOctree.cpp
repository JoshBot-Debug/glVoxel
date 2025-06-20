#include <cmath>
#include <iostream>

#include "SparseVoxelOctree.h"
#include "Voxel/GreedyMesh32.h"
#include "Voxel/GreedyMesh64.h"
#include "Voxel/GreedyMeshi256.h"

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

void SparseVoxelOctree::setBlock(uint64_t (&mask)[], Voxel *voxel) {
  for (int z = 0; z < m_Size; z += 64)
    for (int x = 0; x < m_Size; x += 64)
      for (int y = 0; y < m_Size; y += 64)
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
        setBlock(mask, x + dx, y + dy, z + dz, voxel, half);
}

void SparseVoxelOctree::set(glm::vec3 position, Voxel *voxel, int maxSize) {
  set(static_cast<int>(position.x), static_cast<int>(position.y),
      static_cast<int>(position.z), voxel, maxSize);
}

void SparseVoxelOctree::set(int x, int y, int z, Voxel *voxel, int maxSize) {
  set(m_Root, x, y, z, voxel, m_Size, maxSize);
}

Node *SparseVoxelOctree::get(glm::vec3 position, uint8_t maxDepth,
                             Voxel *filter) {
  return get(static_cast<int>(position.x), static_cast<int>(position.y),
             static_cast<int>(position.z), maxDepth, filter);
}

Node *SparseVoxelOctree::get(int x, int y, int z, uint8_t maxDepth,
                             Voxel *filter) {
  return get(m_Root, x, y, z, m_Size, maxDepth, filter);
}

void SparseVoxelOctree::set(Node *node, int x, int y, int z, Voxel *voxel,
                            int size, int maxSize) {
  if (size == maxSize) {
    node->voxel = voxel;

    for (const Voxel *v : m_UniqueVoxels)
      if (v == voxel)
        return;

    m_UniqueVoxels.push_back(voxel);
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
    const glm::ivec3 np = {floorDiv(x, m_Size) + m_GlobalPosition.x,
                           floorDiv(y, m_Size) + m_GlobalPosition.y,
                           floorDiv(z, m_Size) + m_GlobalPosition.z};

    auto it = m_Neighbours.find(np);

    if (it == m_Neighbours.end() || it->second == nullptr)
      return nullptr;

    return it->second->get({mod(x, m_Size), mod(y, m_Size), mod(z, m_Size)},
                           maxDepth, filter);
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
  if (!m_Root)
    return;

  m_Root->clear();
}

void SparseVoxelOctree::greedyMesh(std::vector<Vertex> &vertices,
                                   Voxel *filter) {
  // const int chunkSize = GreedyMeshi256::s_CHUNK_SIZE;
  // const int chunksPerAxis = std::max(1, m_Size / chunkSize);

  // for (int cz = 0; cz < chunksPerAxis; cz++)
  //   for (int cy = 0; cy < chunksPerAxis; cy++)
  //     for (int cx = 0; cx < chunksPerAxis; cx++)
  //       GreedyMeshi256::Octree(this, vertices, cx * chunkSize, cy *
  //       chunkSize,
  //                              cz * chunkSize, 0, filter);

  const int chunkSize1 = GreedyMesh64::CHUNK_SIZE;
  const int chunksPerAxis1 = std::max(1, m_Size / chunkSize1);

  for (int cz = 0; cz < chunksPerAxis1; cz++)
    for (int cy = 0; cy < chunksPerAxis1; cy++)
      for (int cx = 0; cx < chunksPerAxis1; cx++)
        GreedyMesh64::Octree(this, vertices, cx * chunkSize1, cy *
        chunkSize1,
                               cz * chunkSize1, 0, filter);
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

size_t SparseVoxelOctree::getTotalMemoryUsage() {
  return sizeof(SparseVoxelOctree) + getMemoryUsage(m_Root);
}

const std::vector<Voxel *> &SparseVoxelOctree::getUniqueVoxels() const {
  return m_UniqueVoxels;
}

void SparseVoxelOctree::setNeighbours(
    const glm::ivec3 &globalPosition,
    std::unordered_map<glm::ivec3, SparseVoxelOctree *> &chunks) {
  this->m_GlobalPosition = globalPosition;

  m_Neighbours.clear();
  m_Neighbours.reserve(NEIGHBOUR_DIRECTIONS.size());

  for (const glm::ivec3 &dir : NEIGHBOUR_DIRECTIONS) {
    glm::ivec3 np = m_GlobalPosition + dir;
    if (chunks.contains(np))
      m_Neighbours[np] = chunks.at(np);
    else
      m_Neighbours[np] = nullptr;
  }
}