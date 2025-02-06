#pragma once

#include <vector>

#include "Types.h"

enum class FaceType : uint8_t {
  TOP,
  BOTTOM,
  LEFT,
  RIGHT,
  FRONT,
  BACK
};

class Face
{
public:
  static void Top(std::vector<Vertex> &vertices, float px, float py, float pz, float sx, float sy, float sz);
  static void Bottom(std::vector<Vertex> &vertices, float px, float py, float pz, float sx, float sy, float sz);
  static void Left(std::vector<Vertex> &vertices, float px, float py, float pz, float sx, float sy, float sz);
  static void Right(std::vector<Vertex> &vertices, float px, float py, float pz, float sx, float sy, float sz);
  static void Front(std::vector<Vertex> &vertices, float px, float py, float pz, float sx, float sy, float sz);
  static void Back(std::vector<Vertex> &vertices, float px, float py, float pz, float sx, float sy, float sz);
};