#pragma once
#include <glm/glm.hpp>

struct IVec2Hash {
  size_t operator()(const glm::ivec2 &k) const noexcept {
    size_t h1 = std::hash<int>()(k.x);
    size_t h2 = std::hash<int>()(k.y);
    return h1 ^ (h2 << 1);
  }
};

struct IVec2Equal {
  bool operator()(const glm::ivec2 &lhs, const glm::ivec2 &rhs) const noexcept {
    return lhs.x == rhs.x && lhs.y == rhs.y;
  }
};
