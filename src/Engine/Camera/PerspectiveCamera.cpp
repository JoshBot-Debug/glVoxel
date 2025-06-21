#include "PerspectiveCamera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

float toDegree(float degree) {
  return glm::mod(glm::abs(degree), 360.0f) * (degree < 0 ? -1 : 1);
}

void PerspectiveCamera::update() {
  front.x = sin(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
  front.y = sin(glm::radians(rotation.x));
  front.z = -cos(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));

  front = glm::normalize(front);

  right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
  up = glm::normalize(glm::cross(right, front));

  roll = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), front);

  right = glm::normalize(glm::vec3(roll * glm::vec4(right, 0.0f)));
  up = glm::normalize(glm::vec3(roll * glm::vec4(up, 0.0f)));

  view = glm::lookAt(position, position + front, up);

  projection = glm::perspective(glm::radians(fov),
                                glm::max(viewportWidth / viewportHeight, 1.0f),
                                nearPlane, farPlane);
}

void PerspectiveCamera::setViewportSize(const glm::vec2 &size) {
  viewportWidth = size.x;
  viewportHeight = size.y;
}

void PerspectiveCamera::setViewportSize(float width, float height) {
  viewportWidth = width;
  viewportHeight = height;
}

void PerspectiveCamera::setPosition(float x, float y, float z) {
  position.x = x;
  position.y = y;
  position.z = z;
}

void PerspectiveCamera::setRotation(float pitch, float yaw, float roll) {
  rotation.x = toDegree(pitch);
  rotation.y = toDegree(yaw);
  rotation.z = toDegree(roll);
}

void PerspectiveCamera::translate(float deltaX, float deltaY, float deltaZ) {
  position += deltaX * right;
  position += deltaY * up;
  position += deltaZ * front;
}

void PerspectiveCamera::rotate(float deltaPitch, float deltaYaw,
                               float deltaRoll) {
  rotation.x = toDegree(rotation.x + deltaPitch);
  rotation.y = toDegree(rotation.y + deltaYaw);
  rotation.z = toDegree(rotation.z + deltaRoll);
}

void PerspectiveCamera::setProjection(float fov, float nearPlane,
                                      float farPlane) {
  this->fov = fov;
  this->nearPlane = nearPlane;
  this->farPlane = farPlane;
}

const glm::mat4 PerspectiveCamera::getViewProjectionMatrix() const {
  return projection * view;
}

const glm::mat4 PerspectiveCamera::getProjectionMatrix() const {
  return projection;
}

const glm::mat4 PerspectiveCamera::getViewMatrix() const { return view; }

const glm::vec3 PerspectiveCamera::getRayDirection(int pixelX,
                                                   int pixelY) const {
  // 1. Convert pixel to Normalized Device Coordinates [-1, 1]
  float ndcX = (2.0f * pixelX) / viewportWidth - 1.0f;
  float ndcY = 1.0f - (2.0f * pixelY) / viewportHeight;

  // 2. Clip space position
  glm::vec4 clipCoords(ndcX, ndcY, -1.0f, 1.0f); // -1 for near plane

  // 3. Inverse of view-projection
  glm::mat4 invVP = glm::inverse(projection * view);

  // 4. Unproject to world space
  glm::vec4 worldCoords = invVP * clipCoords;
  worldCoords /= worldCoords.w;

  // 5. Ray direction = from camera position to world point
  glm::vec3 rayDir = glm::normalize(glm::vec3(worldCoords) - position);
  return rayDir;
}