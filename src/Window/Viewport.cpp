#include "Viewport.h"

#include <GL/glew.h>
#include <algorithm>

#include "imgui.h"

Viewport::~Viewport() {}

void Viewport::setTitle(const char *title) { this->title = title; }

void Viewport::setDimensions(glm::vec2 dimensions) {
  this->dimensions = dimensions;
}

void Viewport::resize(glm::vec2 size) {
  if (this->texture) {
    glBindTexture(GL_TEXTURE_2D, this->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  this->setDimensions(size);
  this->onResize(size);
}

void Viewport::createFrameBuffer() {
  // Create & bind the frame buffer
  glGenFramebuffers(1, &this->framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer);

  // Create a texture to render to & bind it
  glGenTextures(1, &this->texture);
  glBindTexture(GL_TEXTURE_2D, this->texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->dimensions.x, this->dimensions.y,
               0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Attach the texture to the framebuffer
  // The texture will now serve as the output for any rendering done to this
  // framebuffer
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         this->texture, 0);

  // Unbind texture & framebuffer
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Viewport::onDraw() {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
  ImGui::Begin(this->title, nullptr, ImGuiWindowFlags_NoCollapse);

  ImVec2 size = ImGui::GetContentRegionAvail();
  ImVec2 position = ImGui::GetWindowPos();
  this->position.x = position.x;
  this->position.y = position.y;

  if (this->dimensions.x != size.x || this->dimensions.y != size.y)
    this->resize({size.x, size.y});

  if (this->framebuffer == 0)
    this->createFrameBuffer();

  glViewport(0, 0, size.x, size.y);
  glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer);

  this->onDrawViewport();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  ImGui::Image((ImTextureID)this->texture,
               ImVec2(this->dimensions.x, this->dimensions.y));

  ImGui::End();
  ImGui::PopStyleVar();
}

glm::vec2 *Viewport::getDimensions() { return &this->dimensions; }

glm::vec2 *Viewport::getPosition() { return &this->position; }