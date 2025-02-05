#include <iostream>
#include <unordered_map>
#include <glm/glm.hpp>
#include "Chunk.h"
#include "Utility.h"

class ChunkManager
{
public:
  static constexpr const uint32_t CHUNKS = 8;

private:
  std::unordered_map<glm::ivec3, Chunk, IVec3Hash> chunks;

public:
  void set(int x, int y, int z, unsigned int value)
  {
    glm::ivec3 coord{std::floor(x / Chunk::SIZE), std::floor(y / Chunk::SIZE), std::floor(z / Chunk::SIZE)};
    Chunk &chunk = chunks[coord];
    chunk.setRootCoordinate(coord);
    chunk.set(std::abs(x) % Chunk::SIZE, std::abs(y) % Chunk::SIZE, std::abs(z) % Chunk::SIZE, value);
  }

  Chunk &get(int x, int y, int z)
  {
    return chunks[{std::floor(x / Chunk::SIZE), std::floor(y / Chunk::SIZE), std::floor(z / Chunk::SIZE)}];
  }

  void update(std::vector<Vertex> &vertices)
  {
    vertices.clear();

    for (auto &[coord, chunk] : chunks)
      chunk.mesh(vertices);
  }

  void clear()
  {
    for (auto &[coord, chunk] : chunks)
      chunk.clear();
  }

  void clear(unsigned int x, unsigned int y, unsigned int z)
  {
    chunks[{x, y, z}].clear();
  }
};