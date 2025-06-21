#pragma once

#include "Engine/Types.h"
#include <array>
#include <glm/glm.hpp>
#include <mutex>
#include <vector>

namespace RaytracerCPU
{
  
  class CVoxelBuffer {
  private:
    static constexpr uint8_t m_Next[2] = {1, 0};
  
  private:
    bool m_Dirty = false;
    std::shared_mutex m_Mutex;
  
    uint8_t m_Current;

    std::array<std::vector<uint32_t>, 2> m_Buffer;
  
  public:
    CVoxelBuffer() = default;
  
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
  };
  
} // namespace RaytracerCPU
