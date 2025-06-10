#include "Face.h"

void Face::Top(std::vector<Vertex> &vertices, float px, float py, float pz, float sx, float sy, float sz)
{
  vertices.emplace_back(Vertex{px, py + sy, pz, 0, 1, 0});
  vertices.emplace_back(Vertex{px + sx, py + sy, pz + sz, 0, 1, 0});
  vertices.emplace_back(Vertex{px + sx, py + sy, pz, 0, 1, 0});

  vertices.emplace_back(Vertex{px, py + sy, pz, 0, 1, 0});
  vertices.emplace_back(Vertex{px, py + sy, pz + sz, 0, 1, 0});
  vertices.emplace_back(Vertex{px + sx, py + sy, pz + sz, 0, 1, 0});
}

void Face::Bottom(std::vector<Vertex> &vertices, float px, float py, float pz, float sx, float sy, float sz)
{
  vertices.emplace_back(Vertex{px, py, pz, 0, -1, 0});
  vertices.emplace_back(Vertex{px + sx, py, pz, 0, -1, 0});
  vertices.emplace_back(Vertex{px + sx, py, pz + sz, 0, -1, 0});

  vertices.emplace_back(Vertex{px, py, pz, 0, -1, 0});
  vertices.emplace_back(Vertex{px + sx, py, pz + sz, 0, -1, 0});
  vertices.emplace_back(Vertex{px, py, pz + sz, 0, -1, 0});
}

void Face::Front(std::vector<Vertex> &vertices, float px, float py, float pz, float sx, float sy, float sz)
{
  vertices.emplace_back(Vertex{px, py, pz, 0, 0, -1});
  vertices.emplace_back(Vertex{px + sx, py + sy, pz, 0, 0, -1});
  vertices.emplace_back(Vertex{px + sx, py, pz, 0, 0, -1});

  vertices.emplace_back(Vertex{px, py, pz, 0, 0, -1});
  vertices.emplace_back(Vertex{px, py + sy, pz, 0, 0, -1});
  vertices.emplace_back(Vertex{px + sx, py + sy, pz, 0, 0, -1});
}

void Face::Back(std::vector<Vertex> &vertices, float px, float py, float pz, float sx, float sy, float sz)
{
  vertices.emplace_back(Vertex{px, py, pz + sz, 0, 0, 1});
  vertices.emplace_back(Vertex{px + sx, py, pz + sz, 0, 0, 1});
  vertices.emplace_back(Vertex{px + sx, py + sy, pz + sz, 0, 0, 1});

  vertices.emplace_back(Vertex{px, py, pz + sz, 0, 0, 1});
  vertices.emplace_back(Vertex{px + sx, py + sy, pz + sz, 0, 0, 1});
  vertices.emplace_back(Vertex{px, py + sy, pz + sz, 0, 0, 1});
}
void Face::Left(std::vector<Vertex> &vertices, float px, float py, float pz, float sx, float sy, float sz)
{
  vertices.emplace_back(Vertex{px, py, pz, -1, 0, 0});
  vertices.emplace_back(Vertex{px, py, pz + sz, -1, 0, 0});
  vertices.emplace_back(Vertex{px, py + sy, pz + sz, -1, 0, 0});

  vertices.emplace_back(Vertex{px, py, pz, -1, 0, 0});
  vertices.emplace_back(Vertex{px, py + sy, pz + sz, -1, 0, 0});
  vertices.emplace_back(Vertex{px, py + sy, pz, -1, 0, 0});
}

void Face::Right(std::vector<Vertex> &vertices, float px, float py, float pz, float sx, float sy, float sz)
{
  vertices.emplace_back(Vertex{px + sx, py, pz, 1, 0, 0});
  vertices.emplace_back(Vertex{px + sx, py + sy, pz + sz, 1, 0, 0});
  vertices.emplace_back(Vertex{px + sx, py, pz + sz, 1, 0, 0});

  vertices.emplace_back(Vertex{px + sx, py, pz, 1, 0, 0});
  vertices.emplace_back(Vertex{px + sx, py + sy, pz, 1, 0, 0});
  vertices.emplace_back(Vertex{px + sx, py + sy, pz + sz, 1, 0, 0});
}
