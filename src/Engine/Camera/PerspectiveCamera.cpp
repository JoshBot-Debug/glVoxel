#include "PerspectiveCamera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

float toDegree(float degree)
{
  return glm::mod(glm::abs(degree), 360.0f) * (degree < 0 ? -1 : 1);
}

void PerspectiveCamera::update()
{
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

  projection = glm::perspective(glm::radians(fov), glm::max(width / height, 1.0f), nearPlane, farPlane);
}

void PerspectiveCamera::setViewportSize(const glm::vec2 &size)
{
  this->width = size.x;
  this->height = size.y;
}

void PerspectiveCamera::setViewportSize(float width, float height)
{
  this->width = width;
  this->height = height;
}

void PerspectiveCamera::setPosition(float x, float y, float z)
{
  position.x = x;
  position.y = y;
  position.z = z;
}

void PerspectiveCamera::setRotation(float pitch, float yaw, float roll)
{
  rotation.x = toDegree(pitch);
  rotation.y = toDegree(yaw);
  rotation.z = toDegree(roll);
}

void PerspectiveCamera::translate(float deltaX, float deltaY, float deltaZ)
{
  position += deltaX * right;
  position += deltaY * up;
  position += deltaZ * front;
}

void PerspectiveCamera::rotate(float deltaPitch, float deltaYaw, float deltaRoll)
{
  rotation.x = toDegree(rotation.x + deltaPitch);
  rotation.y = toDegree(rotation.y + deltaYaw);
  rotation.z = toDegree(rotation.z + deltaRoll);
}

void PerspectiveCamera::setProjection(float fov, float nearPlane, float farPlane)
{
  this->fov = fov;
  this->nearPlane = nearPlane;
  this->farPlane = farPlane;
}

const glm::mat4 PerspectiveCamera::getViewProjectionMatrix() const
{
  return projection * view;
}

const glm::mat4 PerspectiveCamera::getProjectionMatrix() const
{
  return projection;
}

const glm::mat4 PerspectiveCamera::getViewMatrix() const
{
  return view;
}