#pragma once

#include "Engine/Types.h"
#include <glm/glm.hpp>
#include <mutex>
#include <vector>

class CVoxelBuffer {
private:
  bool _dirty = false;
  std::shared_mutex mutex;
  size_t size = 0;
  std::vector<Vertex> vertices;
  std::unordered_map<glm::ivec3, std::vector<Vertex>> chunkVertices;

public:
  CVoxelBuffer() = default;

  void setVertices(const glm::ivec3 &coord, const std::vector<Vertex> &data) {
    std::unique_lock lock(mutex);
    size += data.size();
    chunkVertices[coord].insert(chunkVertices[coord].begin(),
                                std::make_move_iterator(data.begin()),
                                std::make_move_iterator(data.end()));
  }

  const std::vector<Vertex> &getVertices() {
    std::unique_lock lock(mutex);

    for (auto &[k, v] : chunkVertices)
      vertices.insert(vertices.end(), v.begin(), v.end());

    return vertices;
  }

  const size_t getSize() { return size; }

  void clearVertices(const glm::ivec3 &coord) {
    std::unique_lock lock(mutex);
    if (!chunkVertices.contains(coord))
      return;
    size -= chunkVertices.at(coord).size();
    chunkVertices.erase(coord);
  }

  void clean() {
    std::unique_lock lock(mutex);
    vertices.clear();
    _dirty = false;
  }

  void flush() {
    std::unique_lock lock(mutex);
    _dirty = true;
  }

  const bool isDirty() const { return _dirty; }
};