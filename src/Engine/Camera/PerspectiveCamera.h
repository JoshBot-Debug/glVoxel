#pragma once

#include <string>
#include <glm/glm.hpp>

#include "Camera.h"

class PerspectiveCamera : public Camera
{
private:
  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 projection = glm::mat4(0.0f);

  glm::vec3 front = glm::vec3(0.0f);
  glm::vec3 right = glm::vec3(0.0f);
  glm::vec3 up = glm::vec3(0.0f);
  glm::mat4 roll = glm::mat4(0.0f);

public:
  float width = 1.0f;
  float height = 1.0f;

  glm::vec3 rotation = glm::vec3(0.0f);
  glm::vec3 position = glm::vec3(0.0f);

  float fov = 45.0f;
  float farPlane = 100.0f;
  float nearPlane = 0.1f;

  void update() override;

  void setViewportSize(const glm::vec2 &size) override;

  void setViewportSize(float width, float height) override;

  void setPosition(float x, float y, float z) override;

  void setRotation(float pitch, float yaw, float roll) override;

  void setProjection(float fov, float nearPlane, float farPlane);

  void translate(float deltaX, float deltaY, float deltaZ) override;

  void rotate(float deltaPitch, float deltaYaw, float deltaRoll) override;

  const glm::mat4 getViewProjectionMatrix() const override;
  const glm::mat4 getProjectionMatrix() const override;
  const glm::mat4 getViewMatrix() const override;
};