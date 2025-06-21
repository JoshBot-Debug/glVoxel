#pragma once

#include <algorithm>
#include <execution>
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

#include "Debug.h"
#include "Engine/Types.h"
#include "Voxel/Common.h"
#include "Voxel/Node.h"
#include "Voxel/Voxel.h"

class SparseVoxelOctree {
private:
  /**
   * The total side length of the root node's region.
   * For example, 256 means the root covers a 256×256×256 volume.
   */
  int m_Size;

  /**
   * The depth of the octree, typically std::log2(m_Size).
   * Indicates how many levels of subdivision exist.
   */
  uint8_t m_Depth;

  /**
   * Pointer to the root node of the Sparse Voxel Octree.
   * Should never be empty, is always initialized in the constructor.
   */
  Node *m_Root = nullptr;

  /**
   * The coordinate of this SVO chunk in the global grid of chunks.
   * Used for resolving neighbor lookups.
   *
   * Example:
   *
   *   (0,0,0) => center
   *   (1,0,0) => center right
   *   (-1,0,0) => center left
   */
  glm::ivec3 m_ChunkCoord{0, 0, 0};

  /**
   * Map of neighboring SVO chunks keyed by their relative chunk-grid position.
   * For example: (1, 0, 0) → right neighbor, (-1, 0, 0) → left neighbor, etc.
   * Enables out-of-bounds lookups across adjacent SVOs.
   */
  std::unordered_map<glm::ivec3, SparseVoxelOctree *> m_Neighbours;

private:
  /**
   * Internal recursive setter that applies a voxel to all positions marked in
   * the bitmask. Called by the public `set(mask, voxel)` method.
   *
   * @param mask   A bitmask indicating which voxels to set.
   * @param x,y,z  The origin (offset) position in voxel-space for this mask
   * block.
   * @param voxel  The voxel type to set at the marked positions.
   * @param size   The current size of the region being processed.
   */
  void set(uint64_t (&mask)[], int x, int y, int z, Voxel *voxel, int size);

  /**
   * Internal recursive setter that traverses and builds the tree as needed.
   * Called by the public `set(x, y, z, voxel)` and `set(vec3, voxel)` methods.
   *
   * @param node      Current node in the octree.
   * @param x,y,z     Local voxel-space coordinates at this level.
   * @param voxel     The voxel to assign at the final leaf.
   * @param leafSize  The target size of a leaf node (typically 1).
   * @param size      The size of the region represented by this node.
   */
  void set(Node *node, int x, int y, int z, Voxel *voxel, int leafSize,
           int size);

  /**
   * Internal recursive getter that traverses the tree to find a voxel at the
   * given position. Called by the public `get(x, y, z)` and `get(vec3)`
   * methods.
   *
   * @param node    Current node in the octree.
   * @param x,y,z   Local voxel-space coordinates at this level.
   * @param size    The size of the region represented by this node.
   * @param filter  Optional filter; only returns nodes matching this voxel
   * type.
   * @return        Pointer to the node at the target position, or nullptr if
   * not found or filtered out.
   */
  Node *get(Node *node, int x, int y, int z, int size, Voxel *filter = nullptr);

  /**
   * Returns the total memory used in bytes by this node and all it's children.
   */
  size_t getMemoryUsage(Node *node);

  /**
   * Performs floor division of a by b.
   * This ensures correct behavior for negative dividends, returning the
   * mathematical floor of the division instead of truncating toward zero.
   *
   * Examples:
   *   floorDiv( 5,  2) ->  2
   *   floorDiv(-5,  2) -> -3
   *   floorDiv( 5, -2) -> -3
   *   floorDiv(-5, -2) ->  2
   */
  int floorDiv(int a, int b);

  /**
   * Computes the true modulo (remainder) of a divided by b.
   * Unlike the default % operator in C++, this ensures the result is always
   * non-negative and in the range [0, b), even when a is negative.
   *
   * Examples:
   *   mod( 5,  2) -> 1
   *   mod(-5,  2) -> 1
   *   mod( 5, -2) -> -1
   *   mod(-5, -2) -> -1
   */
  int mod(int a, int b);

  Voxel *rayTrace(Node *node, const glm::vec3 &origin, const glm::vec3 &direction, glm::vec3 nodeMin, int size);

public:
  /**
   * Constructs an empty Sparse Voxel Octree with default settings.
   */
  SparseVoxelOctree();

  /**
   * Constructs a Sparse Voxel Octree with the given spatial size.
   *
   * @param size  The side length of the root node's region (e.g., 256 for a
   * 256³ volume).
   */
  SparseVoxelOctree(int size);

  /**
   * Destroys the Sparse Voxel Octree and frees all allocated nodes.
   */
  ~SparseVoxelOctree();

  /**
   * Returns the total side length (width, height, depth) of the root node.
   *
   * @return The root node's spatial size (e.g., 256).
   */
  int getSize();

  /**
   * Returns the maximum depth of the octree.
   * This is computed based on the size: std::log2(size)
   *
   * @return Depth of the tree in number of subdivisions.
   */
  int getDepth();

  /**
   * Returns a pointer to the root node of the octree.
   *
   * @return Root node of the SVO.
   */
  Node *getRoot();

  /**
   * Efficiently sets multiple voxels in the SVO using a 3D bitmask.
   *
   * This method is optimized for bulk voxel insertion and is significantly
   * faster than calling `set()` for every individual voxel. Instead of
   * iterating over x, y, z coordinates and inserting one voxel at a time, this
   * function uses a bitmask to mark the positions of voxels to be set and
   * inserts them efficiently with minimal tree traversal.
   *
   * The user is responsible for creating and populating the bitmask before
   * calling this method. The bitmask should be in a flattened 3D format
   * (x + size * (z + size * y)), with one bit representing each voxel.
   *
   * Example usage:
   *
   *   const int ChunkSize = 256;
   *   uint64_t mask[(ChunkSize * ChunkSize) * (ChunkSize / 64)] = {0};
   *
   *   for (int z = 0; z < ChunkSize; z++)
   *     for (int x = 0; x < ChunkSize; x++)
   *        for (int y= 0; y < ChunkSize; y++)
   *        {
   *            int index = x + size * (z + (size * y));
   *
   *            if(blockIsGrass)                            // Your condition
   *              mask[index / 64] |= 1UL << (index % 64);  // Turn on bit
   *        }
   *
   *   tree.set(mask, grassVoxel);
   *
   * @param mask   A bitmask indicating which voxels to set (1 = set, 0 = skip).
   * @param voxel  Pointer to the voxel type to set at the marked positions.
   */
  void set(uint64_t (&mask)[], Voxel *voxel);

  /**
   * Sets a voxel at the given 3D world position.
   *
   * @param position  The world-space position to place the voxel at.
   * @param voxel     The voxel data to insert.
   * @param leafSize  The smallest voxel size (default is 1 unit).
   */
  void set(glm::vec3 position, Voxel *voxel, int leafSize = 1);

  /**
   * Retrieves the node at the given 3D world position.
   *
   * @param position  The world-space position to query.
   * @param filter    Optional filter; if provided, only matching voxels are
   * returned.
   * @return          Pointer to the node at that position, or nullptr if not
   * found or filtered out.
   */
  Node *get(glm::vec3 position, Voxel *filter = nullptr);

  /**
   * Sets a voxel at the given 3D world position.
   *
   * @param x, y, z   The world-space position to place the voxel at.
   * @param voxel     The voxel data to insert.
   * @param leafSize  The smallest voxel size (default is 1 unit).
   */
  void set(int x, int y, int z, Voxel *voxel, int leafSize = 1);

  /**
   * Retrieves the node at the given 3D world position.
   *
   * @param x, y, z   The world-space position to query.
   * @param filter    Optional filter; if provided, only matching voxels are
   * returned.
   * @return          Pointer to the node at that position, or nullptr if not
   * found or filtered out.
   */
  Node *get(int x, int y, int z, Voxel *filter = nullptr);

  /**
   * Calls node.clear() on the root node.
   * This clears the root node and all it's children.
   * The root node will not be destroyed, only cleared. All children will be
   * cleared and deleted.alignas The root node will be destroyed once the
   * destructor of the SVO is called.
   */
  void clear();

  /**
   * Set the neighbouring SVO chunks.
   * This is done so that when you call tree.get(), if the position is out of
   * this SVO, it will automatically lookup in the correct neighbouring SVO.
   *
   * Neighbours are keyed by their global position relative to each other.
   * (0,0,0) => center
   * (1,0,0) => center right
   * (-1,0,0) => center left
   * ...etc.
   *
   * @param chunkCoord The position of this SVO chunks. (-1,0,0) | (0,0,0) |
   * (1,0,0) | ...
   * @param neighbours An unordered map that contains pointers to all SVO
   * chunks, keyed by their chunkCoord.
   */
  void setNeighbours(
      const glm::ivec3 &chunkCoord,
      const std::unordered_map<glm::ivec3, SparseVoxelOctree *> &neighbours);

  /**
   * Returns the total memory usage of the SVO in bytes.
   * This does not include neighbours.
   */
  size_t getTotalMemoryUsage();


  Voxel *rayTrace(const glm::vec3 &origin, const glm::vec3 &direction);
};