#pragma once

#include <glm/glm.hpp>

struct IVec3Hash
{
  std::size_t operator()(const glm::ivec3 &v) const
  {
    return static_cast<std::size_t>(
        v.x * 73856093 ^ v.y * 19349663 ^ v.z * 83492791);
  }
};
