#include "Input.h"

GLFWwindow *Input::s_Window = nullptr;

void Input::SetWindowContext(GLFWwindow *window) { Input::s_Window = window; }

const glm::vec2 Input::MousePosition() {
  double x, y;
  glfwGetCursorPos(s_Window, &x, &y);
  return glm::vec2(x, y);
}