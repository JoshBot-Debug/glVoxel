#include <unordered_map>
#include <glm/glm.hpp>
#include "VoxelChunk.h"

class ChunkManager
{
private:
  std::unordered_map<glm::ivec3, VoxelChunk, IVec3Hash> chunks;

public:
  ChunkManager() {}
  ~ChunkManager() {}
};