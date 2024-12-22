#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Voxel
{
  unsigned int type = 0;
  glm::mat4 identity = glm::mat4(1.0f);

  void setPosition(glm::vec3 position)
  {
    identity = glm::translate(glm::mat4(1.0f), position);
  }

  void setRotation(glm::vec3 rotation)
  {
    identity = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    identity = glm::rotate(identity, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    identity = glm::rotate(identity, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
  }

  void setScale(glm::vec3 scale)
  {
    identity = glm::scale(glm::mat4(1.0f), scale);
  }

  void translate(float deltaX, float deltaY, float deltaZ)
  {
    identity = glm::translate(identity, glm::vec3(deltaX, deltaY, deltaZ));
  }

  void translate(glm::vec3 delta)
  {
    identity = glm::translate(identity, delta);
  }

  void scale(float deltaX, float deltaY, float deltaZ)
  {
    identity = glm::scale(identity, glm::vec3(deltaX, deltaY, deltaZ));
  }

  void scale(glm::vec3 delta)
  {
    identity = glm::scale(identity, delta);
  }

  void rotate(float deltaX, float deltaY, float deltaZ)
  {
    identity = glm::rotate(identity, glm::radians(deltaX), glm::vec3(1.0f, 0.0f, 0.0f));
    identity = glm::rotate(identity, glm::radians(deltaY), glm::vec3(0.0f, 1.0f, 0.0f));
    identity = glm::rotate(identity, glm::radians(deltaZ), glm::vec3(0.0f, 0.0f, 1.0f));
  }

  void rotate(glm::vec3 delta)
  {
    identity = glm::rotate(identity, glm::radians(delta.x), glm::vec3(1.0f, 0.0f, 0.0f));
    identity = glm::rotate(identity, glm::radians(delta.y), glm::vec3(0.0f, 1.0f, 0.0f));
    identity = glm::rotate(identity, glm::radians(delta.z), glm::vec3(0.0f, 0.0f, 1.0f));
  }
};