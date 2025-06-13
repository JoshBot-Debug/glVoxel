#include "Input.h"

GLFWwindow *Input::window = nullptr;

void Input::SetWindowContext(GLFWwindow *window) { Input::window = window; }

const glm::vec2 Input::MousePosition() {
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  return glm::vec2(x, y);
}