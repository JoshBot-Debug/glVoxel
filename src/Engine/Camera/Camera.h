#pragma once

#include <glm/glm.hpp>

class Camera {
public:
  virtual void update() = 0;

  virtual void setViewportSize(const glm::vec2 &size) = 0;

  virtual void setViewportSize(float width, float height) = 0;

  virtual void setPosition(float x, float y, float z) = 0;

  virtual void setRotation(float pitch, float yaw, float roll) = 0;

  virtual void setProjection(float fov, float nearPlane, float farPlane) = 0;

  virtual void translate(float deltaX, float deltaY, float deltaZ) = 0;

  virtual void rotate(float deltaPitch, float deltaYaw, float deltaRoll) = 0;

  virtual const glm::mat4 getViewProjectionMatrix() const = 0;
  virtual const glm::mat4 getProjectionMatrix() const = 0;
  virtual const glm::mat4 getViewMatrix() const = 0;
};