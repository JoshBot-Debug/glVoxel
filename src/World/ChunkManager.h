#include <unordered_map>
#include <glm/glm.hpp>
#include "VoxelChunk.h"

class ChunkManager
{
public:
  static constexpr const unsigned int CHUNKS = 1;

private:
  std::unordered_map<glm::ivec3, VoxelChunk, IVec3Hash> chunks;

public:
  void set(unsigned int x, unsigned int y, unsigned int z, unsigned int value)
  {
    return chunks[{std::floor(x / CHUNKS), std::floor(y / CHUNKS), std::floor(z / CHUNKS)}].set(x % CHUNKS, y % CHUNKS, z % CHUNKS, value);
  }

  void update(std::vector<Vertex> &vertices)
  {
    vertices.clear();

    for (auto &[coord, chunk] : chunks)
      chunk.mesh(vertices);
  }

  void clearAll()
  {
    for (auto &[coord, chunk] : chunks)
      chunk.clear();
  }
};