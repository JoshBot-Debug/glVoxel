#pragma once

#include <vector>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class VertexDraw
{
  STATIC = GL_STATIC_DRAW,
  DYNAMIC = GL_DYNAMIC_DRAW,
  STREAM = GL_STREAM_DRAW
};

enum class VertexType
{
  FLOAT = GL_FLOAT,
  INT = GL_INT,
  UNSIGNED_INT = GL_UNSIGNED_INT,
  BYTE = GL_BYTE,
  UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
  SHORT = GL_SHORT,
  UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
  BOOL = GL_BOOL,
};

enum class Primitive
{
  POINTS = GL_POINTS,
  LINES = GL_LINES,
  LINE_STRIP = GL_LINE_STRIP,
  LINE_LOOP = GL_LINE_LOOP,
  TRIANGLES = GL_TRIANGLES,
  TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
  TRIANGLE_FAN = GL_TRIANGLE_FAN
};

struct Vertex
{
  glm::vec3 position = glm::vec3(0.0f);
  glm::vec3 normal = glm::vec3(0.0f);
  glm::vec2 texCoord = glm::vec2(0.0f);
  glm::vec3 tangent = glm::vec3(0.0f);
  glm::vec3 bitangent = glm::vec3(0.0f);
  glm::vec3 color = glm::vec3(1.0f);

  bool operator==(const Vertex &other) const
  {
    return position == other.position && texCoord == other.texCoord && normal == other.normal;
  }

  std::size_t operator()(const Vertex &v) const
  {
    auto hash1 = std::hash<float>()(v.position.x) ^ (std::hash<float>()(v.position.y) << 1) ^ (std::hash<float>()(v.position.z) << 2);
    auto hash2 = std::hash<float>()(v.texCoord.x) ^ (std::hash<float>()(v.texCoord.y) << 1);
    auto hash3 = std::hash<float>()(v.normal.x) ^ (std::hash<float>()(v.normal.y) << 1) ^ (std::hash<float>()(v.normal.z) << 2);
    auto hash4 = std::hash<float>()(v.tangent.x) ^ (std::hash<float>()(v.tangent.y) << 1) ^ (std::hash<float>()(v.tangent.z) << 2);
    auto hash5 = std::hash<float>()(v.bitangent.x) ^ (std::hash<float>()(v.bitangent.y) << 1) ^ (std::hash<float>()(v.bitangent.z) << 2);
    auto hash6 = std::hash<float>()(v.color.x) ^ (std::hash<float>()(v.color.y) << 1) ^ (std::hash<float>()(v.color.z) << 2);
    return hash1 ^ hash2 ^ hash3 ^ hash4 ^ hash5 ^ hash6;
  }

  bool operator()(const Vertex &a, const Vertex &b) const
  {
    return a.position == b.position && a.texCoord == b.texCoord && a.normal == b.normal && a.tangent == b.tangent && a.bitangent == b.bitangent && a.color == b.color;
  }
};

struct InstanceBuffer
{
  glm::mat4 model = glm::mat4(1.0f);
  glm::mat3 normalMatrix = glm::mat3(1.0f);
  glm::vec3 color = glm::vec3(1.0f);
};

class Instance
{
private:
  InstanceBuffer buffer;

public:
  unsigned int id;
  glm::vec3 translate = glm::vec3(0.0f);
  glm::vec3 rotation = glm::vec3(0.0f);
  glm::vec3 scale = glm::vec3(1.0f);
  glm::vec3 color = glm::vec3(1.0f);

  const InstanceBuffer &update()
  {
    buffer.model = glm::mat4(1.0f);
    buffer.model = glm::scale(buffer.model, scale);
    buffer.model = glm::rotate(buffer.model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    buffer.model = glm::rotate(buffer.model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    buffer.model = glm::rotate(buffer.model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    buffer.model = glm::translate(buffer.model, translate);
    buffer.color = color;
    buffer.normalMatrix = glm::transpose(glm::inverse(glm::mat3(buffer.model)));

    return buffer;
  }
};

struct Mesh
{
  std::string name;
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
};

struct DrawElementsIndirectCommand
{
  unsigned int count;
  unsigned int primCount;
  unsigned int firstIndex;
  unsigned int baseVertex;
  unsigned int baseInstance;
};