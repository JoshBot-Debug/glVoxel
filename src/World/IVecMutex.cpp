#include "IVecMutex.h"

std::shared_mutex &IVecMutex::get(const glm::ivec3 &key) {
  {
    std::shared_lock<std::shared_mutex> readLock(mutex);
    auto it = pool.find(key);
    if (it != pool.end())
      return *it->second;
  }

  std::unique_lock<std::shared_mutex> writeLock(mutex);
  auto &mtxPtr = pool[key];
  if (!mtxPtr) {
    mtxPtr = std::make_unique<std::shared_mutex>();
  }
  return *mtxPtr;
}

void IVecMutex::remove(const glm::ivec3 &key) {
  std::shared_lock<std::shared_mutex> readLock(mutex);
  auto it = pool.find(key);
  if (it != pool.end())
    if (it->second.use_count() == 1)
      pool.erase(key);
}
