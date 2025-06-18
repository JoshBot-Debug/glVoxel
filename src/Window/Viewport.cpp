#include "Viewport.h"

#include <GL/glew.h>
#include <algorithm>

#include "imgui.h"

Viewport::~Viewport() {}

void Viewport::setTitle(const char *title) { m_Title = title; }

void Viewport::setDimensions(glm::ivec2 dimensions) {
  m_Dimensions = dimensions;
}

void Viewport::resize(glm::ivec2 size) {
  if (m_Texture) {
    glBindTexture(GL_TEXTURE_2D, m_Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  setDimensions(size);
  onResize(size);
}

void Viewport::createFrameBuffer() {
  // Create & bind the frame buffer
  glGenFramebuffers(1, &m_Framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);

  // Create a texture to render to & bind it
  glGenTextures(1, &m_Texture);
  glBindTexture(GL_TEXTURE_2D, m_Texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Dimensions.x, m_Dimensions.y, 0,
               GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Attach the texture to the framebuffer
  // The texture will now serve as the output for any rendering done to this
  // framebuffer
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         m_Texture, 0);

  // Unbind texture & framebuffer
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Viewport::onDraw() {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
  ImGui::Begin(m_Title, nullptr, ImGuiWindowFlags_NoCollapse);

  ImVec2 cra = ImGui::GetContentRegionAvail();

  glm::ivec2 size = {cra.x, cra.y};
  ImVec2 position = ImGui::GetWindowPos();
  m_Position.x = position.x;
  m_Position.y = position.y;

  if (m_Dimensions.x != size.x || m_Dimensions.y != size.y)
    resize({size.x, size.y});

  if (m_Framebuffer == 0)
    createFrameBuffer();

  glViewport(0, 0, size.x, size.y);
  glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);

  onDrawViewport();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  ImGui::Image((ImTextureID)m_Texture,
               ImVec2(static_cast<float>(m_Dimensions.x),
                      static_cast<float>(m_Dimensions.y)));

  ImGui::End();
  ImGui::PopStyleVar();
}

glm::ivec2 &Viewport::getDimensions() { return m_Dimensions; }

glm::vec2 &Viewport::getPosition() { return m_Position; }