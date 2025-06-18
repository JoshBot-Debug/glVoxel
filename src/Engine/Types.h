#pragma once

#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class VertexDraw {
  STATIC = GL_STATIC_DRAW,
  DYNAMIC = GL_DYNAMIC_DRAW,
  STREAM = GL_STREAM_DRAW
};

enum class VertexType {
  FLOAT = GL_FLOAT,
  INT = GL_INT,
  UNSIGNED_INT = GL_UNSIGNED_INT,
  BYTE = GL_BYTE,
  UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
  SHORT = GL_SHORT,
  UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
  BOOL = GL_BOOL,
};

enum class Primitive {
  POINTS = GL_POINTS,
  LINES = GL_LINES,
  LINE_STRIP = GL_LINE_STRIP,
  LINE_LOOP = GL_LINE_LOOP,
  TRIANGLES = GL_TRIANGLES,
  TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
  TRIANGLE_FAN = GL_TRIANGLE_FAN
};

struct Vertex {
  float x;
  float y;
  float z;
  float nx;
  float ny;
  float nz;
  unsigned int color;
  unsigned int material;

  Vertex() = default;
  Vertex(float a_X, float a_Y, float a_Z, float a_NX, float a_NY, float a_NZ)
      : x(a_X), y(a_Y), z(a_Z), nx(a_NX), ny(a_NY), nz(a_NZ) {}

  Vertex(float a_X, float a_Y, float a_Z, float a_NX, float a_NY, float a_NZ,
         unsigned int a_Color)
      : x(a_X), y(a_Y), z(a_Z), nx(a_NX), ny(a_NY), nz(a_NZ), color(a_Color) {}

  Vertex(float a_X, float a_Y, float a_Z, float a_NX, float a_NY, float a_NZ,
         unsigned int a_Color, unsigned int a_Material)
      : x(a_X), y(a_Y), z(a_Z), nx(a_NX), ny(a_NY), nz(a_NZ), color(a_Color),
        material(a_Material) {}

  bool operator==(const Vertex &other) const {
    return x == other.x && y == other.y && z == other.z && nx == other.nx &&
           ny == other.ny && nz == other.nz;
  }
};

struct InstanceBuffer {
  glm::mat4 model = glm::mat4(1.0f);
  glm::mat3 normalMatrix = glm::mat3(1.0f);
  glm::vec3 color = glm::vec3(1.0f);
};

class Instance {
private:
  InstanceBuffer buffer;

public:
  unsigned int id;
  glm::vec3 translate = glm::vec3(0.0f);
  glm::vec3 rotation = glm::vec3(0.0f);
  glm::vec3 scale = glm::vec3(1.0f);
  glm::vec3 color = glm::vec3(1.0f);

  const InstanceBuffer &update() {
    buffer.model = glm::mat4(1.0f);
    buffer.model = glm::scale(buffer.model, scale);
    buffer.model = glm::rotate(buffer.model, glm::radians(rotation.x),
                               glm::vec3(1.0f, 0.0f, 0.0f));
    buffer.model = glm::rotate(buffer.model, glm::radians(rotation.y),
                               glm::vec3(0.0f, 1.0f, 0.0f));
    buffer.model = glm::rotate(buffer.model, glm::radians(rotation.z),
                               glm::vec3(0.0f, 0.0f, 1.0f));
    buffer.model = glm::translate(buffer.model, translate);
    buffer.color = color;
    buffer.normalMatrix = glm::transpose(glm::inverse(glm::mat3(buffer.model)));

    return buffer;
  }
};

struct Mesh {
  std::string name;
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
};

struct DrawElementsIndirectCommand {
  unsigned int count;
  unsigned int primCount;
  unsigned int firstIndex;
  unsigned int baseVertex;
  unsigned int baseInstance;
};