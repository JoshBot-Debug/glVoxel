#include "Face.h"

void Face::Top(std::vector<Vertex> &vertices, float px, float py, float pz, float sx, float sy, float sz)
{
  vertices.emplace_back(Vertex{px, py + sy, pz, 0});
  vertices.emplace_back(Vertex{px + sx, py + sy, pz + sz, 0});
  vertices.emplace_back(Vertex{px + sx, py + sy, pz, 0});

  vertices.emplace_back(Vertex{px, py + sy, pz, 0});
  vertices.emplace_back(Vertex{px, py + sy, pz + sz, 0});
  vertices.emplace_back(Vertex{px + sx, py + sy, pz + sz, 0});
}

void Face::Bottom(std::vector<Vertex> &vertices, float px, float py, float pz, float sx, float sy, float sz)
{
  vertices.emplace_back(Vertex{px, py, pz, 1});
  vertices.emplace_back(Vertex{px + sx, py, pz, 1});
  vertices.emplace_back(Vertex{px + sx, py, pz + sz, 1});

  vertices.emplace_back(Vertex{px, py, pz, 1});
  vertices.emplace_back(Vertex{px + sx, py, pz + sz, 1});
  vertices.emplace_back(Vertex{px, py, pz + sz, 1});
}

void Face::Front(std::vector<Vertex> &vertices, float px, float py, float pz, float sx, float sy, float sz)
{
  vertices.emplace_back(Vertex{px, py, pz, 2});
  vertices.emplace_back(Vertex{px + sx, py + sy, pz, 2});
  vertices.emplace_back(Vertex{px + sx, py, pz, 2});

  vertices.emplace_back(Vertex{px, py, pz, 2});
  vertices.emplace_back(Vertex{px, py + sy, pz, 2});
  vertices.emplace_back(Vertex{px + sx, py + sy, pz, 2});
}

void Face::Back(std::vector<Vertex> &vertices, float px, float py, float pz, float sx, float sy, float sz)
{
  vertices.emplace_back(Vertex{px, py, pz + sz, 3});
  vertices.emplace_back(Vertex{px + sx, py, pz + sz, 3});
  vertices.emplace_back(Vertex{px + sx, py + sy, pz + sz, 3});

  vertices.emplace_back(Vertex{px, py, pz + sz, 3});
  vertices.emplace_back(Vertex{px + sx, py + sy, pz + sz, 3});
  vertices.emplace_back(Vertex{px, py + sy, pz + sz, 3});
}
void Face::Left(std::vector<Vertex> &vertices, float px, float py, float pz, float sx, float sy, float sz)
{
  vertices.emplace_back(Vertex{px, py, pz, 4});
  vertices.emplace_back(Vertex{px, py, pz + sz, 4});
  vertices.emplace_back(Vertex{px, py + sy, pz + sz, 4});

  vertices.emplace_back(Vertex{px, py, pz, 4});
  vertices.emplace_back(Vertex{px, py + sy, pz + sz, 4});
  vertices.emplace_back(Vertex{px, py + sy, pz, 4});
}

void Face::Right(std::vector<Vertex> &vertices, float px, float py, float pz, float sx, float sy, float sz)
{
  vertices.emplace_back(Vertex{px + sx, py, pz, 5});
  vertices.emplace_back(Vertex{px + sx, py + sy, pz + sz, 5});
  vertices.emplace_back(Vertex{px + sx, py, pz + sz, 5});

  vertices.emplace_back(Vertex{px + sx, py, pz, 5});
  vertices.emplace_back(Vertex{px + sx, py + sy, pz, 5});
  vertices.emplace_back(Vertex{px + sx, py + sy, pz + sz, 5});
}
