#include "OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>

void OrthographicCamera::update()
{
  float w = width * offsetX;
  float h = height * offsetY;

  projection = glm::ortho(-w, +w, -h, +h, -1.0f, 1.0f);

  view = glm::mat4(1.0f);
  view = glm::translate(view, -position);

  view = glm::rotate(view, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
  view = glm::rotate(view, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
  view = glm::rotate(view, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
}


void OrthographicCamera::setViewportSize(const glm::vec2 &size)
{
  this->width = size.x;
  this->height = size.y;
}

void OrthographicCamera::setViewportSize(float width, float height)
{
  this->width = width;
  this->height = height;
}

void OrthographicCamera::setOffset(float offsetX, float offsetY)
{
  this->offsetX = offsetX;
  this->offsetY = offsetY;
}

void OrthographicCamera::setPosition(float x, float y, float z)
{
  position.x = x - (width * offsetX);
  position.y = y - (height * offsetY);
  position.z = z;
}

void OrthographicCamera::setRotation(float pitch, float yaw, float roll)
{
  rotation.x = pitch;
  rotation.y = yaw;
  rotation.z = roll;
}

void OrthographicCamera::translate(float deltaX, float deltaY, float deltaZ)
{
  position.x += deltaX;
  position.y += deltaY;
  position.z += deltaZ;
}

void OrthographicCamera::rotate(float deltaPitch, float deltaYaw, float deltaRoll)
{
  rotation.x += deltaPitch;
  rotation.y += deltaYaw;
  rotation.z += deltaRoll;
}

const glm::mat4 OrthographicCamera::getViewProjectionMatrix() const
{
  return projection * view;
}

const glm::mat4 OrthographicCamera::getProjectionMatrix() const
{
  return projection;
}

const glm::mat4 OrthographicCamera::getViewMatrix() const
{
  return view;
}
