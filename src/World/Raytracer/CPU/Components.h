#pragma once

#include "Engine/Types.h"
#include <array>
#include <glm/glm.hpp>
#include <mutex>
#include <vector>

namespace RaytracerCPU {

class CTextureBuffer {
private:
  static constexpr uint8_t m_Next[2] = {1, 0};

private:
  bool m_Dirty = false;
  std::shared_mutex m_Mutex;

  uint8_t m_Current = 0;

  std::array<std::vector<uint32_t>, 2> m_Buffer = {};

  glm::ivec2 m_Dimensions = {0, 0};
  std::vector<int> m_DimensionXIter = {};
  std::vector<int> m_DimensionYIter = {};

public:
  CTextureBuffer() = default;

  std::vector<uint32_t> &getUpdateBuffer() {
    return m_Buffer[m_Next[m_Current]];
  }

  const std::vector<uint32_t> &getBuffer() {
    std::shared_lock lock(m_Mutex);
    return m_Buffer[m_Current];
  }

  int getSize() const { return static_cast<int>(m_Buffer[m_Current].size()); }

  void clean() {
    std::unique_lock lock(m_Mutex);
    m_Dirty = false;
  }

  void flush() {
    std::unique_lock lock(m_Mutex);
    m_Dirty = true;

    m_Current = m_Next[m_Current];

    m_Buffer[m_Next[m_Current]].clear();
  }

  bool isDirty() { return m_Dirty; }

  void setDimension(int width, int height) {
    std::unique_lock lock(m_Mutex);

    // if (m_Dimensions.x == width && m_Dimensions.y == height)
    //   return;
      
    m_Dimensions.x = width;
    m_Dimensions.y = height;

    m_DimensionXIter.resize(width);
    for (int i = 0; i < width; i++)
      m_DimensionXIter[i] = i;

    m_DimensionYIter.resize(height);
    for (int i = 0; i < height; i++)
      m_DimensionYIter[i] = i;


    m_Buffer[m_Next[m_Current]].clear();
    m_Buffer[m_Next[m_Current]].resize(width * height);
  }

  const glm::ivec2 &getDimension() { return m_Dimensions; }

  const std::vector<int> &getDimensionXIter() { return m_DimensionXIter; }

  const std::vector<int> &getDimensionYIter() { return m_DimensionYIter; }
};

} // namespace RaytracerCPU
