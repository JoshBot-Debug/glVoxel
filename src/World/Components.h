#pragma once

#include "Engine/Types.h"
#include <array>
#include <glm/glm.hpp>
#include <mutex>
#include <vector>

class CVoxelBuffer {
private:
  bool _dirty = false;
  std::shared_mutex mutex;
  std::vector<Vertex> vertices;

  static constexpr uint8_t next[2] = {1, 0};

  uint8_t current;
  std::array<std::vector<Vertex>, 2> buffer;
  std::unordered_map<glm::ivec3, std::vector<Vertex>> chunkVertices;

public:
  CVoxelBuffer() = default;

  void setVertices(const glm::ivec3 &coord, const std::vector<Vertex> &data) {
    std::unique_lock lock(mutex);
    chunkVertices[coord].insert(chunkVertices[coord].begin(),
                                std::make_move_iterator(data.begin()),
                                std::make_move_iterator(data.end()));
  }

  const std::vector<Vertex> &getVertices() {
    std::shared_lock lock(mutex);
    return buffer[current];
  }

  const size_t getSize() { return buffer[current].size(); }

  void erase(const glm::ivec3 &coord) {
    std::unique_lock lock(mutex);
    if (!chunkVertices.contains(coord))
      return;
    chunkVertices.erase(coord);
  }

  void clean() {
    std::unique_lock lock(mutex);
    _dirty = false;
  }

  void flush() {
    std::unique_lock lock(mutex);
    _dirty = true;

    buffer[next[current]].clear();

    for (auto &[k, v] : chunkVertices)
      buffer[next[current]].insert(buffer[next[current]].end(), v.begin(),
                                   v.end());

    current = next[current];

    buffer[next[current]].clear();
  }

  const bool isDirty() const { return _dirty; }
};