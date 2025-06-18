#pragma once

#include "Engine/Types.h"
#include <array>
#include <glm/glm.hpp>
#include <mutex>
#include <vector>

class CVoxelBuffer {
private:
  static constexpr uint8_t m_Next[2] = {1, 0};

private:
  bool m_Dirty = false;
  std::shared_mutex m_Mutex;

  uint8_t m_Current;
  std::array<std::vector<Vertex>, 2> m_Buffer;
  std::unordered_map<glm::ivec3, std::vector<Vertex>> m_ChunkVertices;

public:
  CVoxelBuffer() = default;

  void setVertices(const glm::ivec3 &coord, const std::vector<Vertex> &data) {
    std::unique_lock lock(m_Mutex);
    m_ChunkVertices[coord].insert(m_ChunkVertices[coord].begin(),
                                std::make_move_iterator(data.begin()),
                                std::make_move_iterator(data.end()));
  }

  const std::vector<Vertex> &getVertices() {
    std::shared_lock lock(m_Mutex);
    return m_Buffer[m_Current];
  }

  int getSize() const { return static_cast<int>(m_Buffer[m_Current].size()); }

  void erase(const glm::ivec3 &coord) {
    std::unique_lock lock(m_Mutex);
    if (!m_ChunkVertices.contains(coord))
      return;
    m_ChunkVertices.erase(coord);
  }

  void clean() {
    std::unique_lock lock(m_Mutex);
    m_Dirty = false;
  }

  void flush() {
    std::unique_lock lock(m_Mutex);
    m_Dirty = true;

    m_Buffer[m_Next[m_Current]].clear();

    for (auto &[k, v] : m_ChunkVertices)
      m_Buffer[m_Next[m_Current]].insert(m_Buffer[m_Next[m_Current]].end(), v.begin(),
                                     v.end());

    m_Current = m_Next[m_Current];

    m_Buffer[m_Next[m_Current]].clear();
  }

  bool isDirty() { return m_Dirty; }
};