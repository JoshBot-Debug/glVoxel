#pragma once

#include <mutex>
#include <memory>
#include <unordered_map>
#include <shared_mutex>
#include <glm/glm.hpp>

#include "Voxel/Common.h"

class IVecMutex {
private:
  std::unordered_map<glm::ivec3, std::shared_ptr<std::shared_mutex>> m_Pool;
  std::shared_mutex m_Mutex;

public:
  std::shared_mutex &get(const glm::ivec3 &key);
  void remove(const glm::ivec3 &key);
};