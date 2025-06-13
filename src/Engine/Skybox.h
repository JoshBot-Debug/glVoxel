#pragma once

#include <string>
#include <vector>

#include "Engine/Camera/Camera.h"
#include "Engine/Core/Buffer.h"
#include "Engine/Core/VertexArray.h"
#include "Engine/Shader.h"

class Skybox {
private:
  Buffer vbo;
  VertexArray vao;
  unsigned int texture = 0;

public:
  /**
   * This order here is important
   *
   * right, left, top, bottom, front, back
   */
  Skybox(std::vector<std::string> faces);
  ~Skybox() = default;

  void draw(const Camera &camera, Shader &shader,
            const std::string &shaderName) const;
  void unbind() const;
};