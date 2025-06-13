#pragma once

#include "InputKey.h"

#include <glm/glm.hpp>
#include <iostream>

class Input {
private:
  static GLFWwindow *window;
  static glm::vec2 scroll;

public:
  static void SetWindowContext(GLFWwindow *window);

  static const glm::vec2 MousePosition();

  static void ResetScroll() {
    scroll.x = 0.0f;
    scroll.y = 0.0f;
  }

  static void ScrollCallback(GLFWwindow *window, double x, double y) {
    scroll.x = x;
    scroll.y = y;
  }

  static const glm::vec2 GetScroll() { return scroll; }

  /**
   * @param key Expects a KeyboardKey or MouseButton
   */
  template <typename T> static const bool KeyPress(const T &key) {
    if (!window)
      return false;

    int state = 0;

    if constexpr (std::is_same_v<T, KeyboardKey>)
      state = glfwGetKey(window, static_cast<int>(key));

    else if constexpr (std::is_same_v<T, MouseButton>)
      state = glfwGetMouseButton(window, static_cast<int>(key));

    return state == static_cast<int>(KeyAction::PRESS);
  }

  /**
   * @param key Expects a KeyboardKey or MouseButton
   */
  template <typename T> static const bool KeyRelease(const T &key) {
    if (!window)
      return false;

    int state = 0;

    if constexpr (std::is_same_v<T, KeyboardKey>)
      state = glfwGetKey(window, static_cast<int>(key));

    else if constexpr (std::is_same_v<T, MouseButton>)
      state = glfwGetMouseButton(window, static_cast<int>(key));

    return state == static_cast<int>(KeyAction::RELEASE);
  }
};

inline glm::vec2 Input::scroll(0.0f);