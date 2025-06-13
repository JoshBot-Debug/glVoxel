#include "Skybox.h"
#include <GL/glew.h>
#include <stb/stb_image.h>

#include "Debug.h"

const std::vector<float> SkyboxCube = {
    -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,
    -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
    -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,
    -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,
    -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f,
    1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

Skybox::Skybox(std::vector<std::string> faces)
    : vbo(BufferTarget::ARRAY_BUFFER, VertexDraw::DYNAMIC) {
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

  int width, height, nrChannels;
  for (unsigned int i = 0; i < faces.size(); i++) {
    unsigned char *data =
        stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
    if (data) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
                   0, GL_RGB, GL_UNSIGNED_BYTE, data);
    } else {
      LOG_BREAK_BEFORE;
      LOG("Skybox: Failed to read skybox face texture");
      LOG_BREAK_AFTER;
    }

    stbi_image_free(data);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

  vao.generate();
  vbo.generate();
  vao.bind();
  vbo.set(SkyboxCube);
  vao.set(0, 3, VertexType::FLOAT, false, 3 * sizeof(float), (void *)0);
  vbo.unbind();
  vao.unbind();
}

void Skybox::draw(const Camera &camera, Shader &shader,
                  const std::string &shaderName) const {
  glDepthFunc(GL_LEQUAL);
  shader.bind(shaderName);

  glm::mat4 view = glm::mat4(glm::mat3(camera.getViewMatrix()));
  shader.setUniformMatrix4fv("u_View", view);
  shader.setUniformMatrix4fv("u_Projection", camera.getProjectionMatrix());

  vao.bind();
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
  glDrawArrays(GL_TRIANGLES, 0, 36);

  glDepthFunc(GL_LESS);
}

void Skybox::unbind() const { glBindTexture(GL_TEXTURE_CUBE_MAP, 0); }
