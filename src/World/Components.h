#pragma once

#include "Engine/Types.h"
#include <mutex>
#include <vector>

class CVoxelBuffer {
private:
  bool _dirty = false;
  std::shared_mutex mutex;
  std::vector<Vertex> vertices;

public:
  CVoxelBuffer() = default;

  void setVertices(const std::vector<Vertex> data) {
    std::unique_lock lock(mutex);
    // vertices.clear();
    vertices.insert(vertices.end(), std::make_move_iterator(data.begin()),
                    std::make_move_iterator(data.end()));
  }

  const std::vector<Vertex> &getVertices() {
    std::unique_lock lock(mutex);
    return vertices;
  }

  const size_t getSize() { return vertices.size(); }

  void clean() {
    std::unique_lock lock(mutex);
    _dirty = false;
  }

  void flush() {
    std::unique_lock lock(mutex);
    _dirty = true;
  }

  const bool isDirty() const { return _dirty; }
};