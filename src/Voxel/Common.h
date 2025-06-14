#pragma once
#include <glm/glm.hpp>

namespace std
{
  template <>
  struct hash<glm::ivec2>
  {
    size_t operator()(const glm::ivec2 &k) const noexcept
    {
      size_t h1 = hash<int>()(k.x);
      size_t h2 = hash<int>()(k.y);
      return h1 ^ (h2 << 1);
    }
  };

  template <>
  struct hash<glm::ivec3>
  {
    size_t operator()(const glm::ivec3 &k) const noexcept
    {
      size_t h1 = hash<int>()(k.x);
      size_t h2 = hash<int>()(k.y);
      size_t h3 = hash<int>()(k.z);
      return h1 ^ (h2 << 1) ^ (h3 << 1);
    }
  };
}