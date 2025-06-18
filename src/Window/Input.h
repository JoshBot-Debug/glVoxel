#pragma once

#include "InputKey.h"

#include <glm/glm.hpp>
#include <iostream>

class Input {
private:
  static GLFWwindow *s_Window;
  static glm::vec2 s_Scroll;

public:
  static void SetWindowContext(GLFWwindow *window);

  static const glm::vec2 MousePosition();

  static void ResetScroll() {
    s_Scroll.x = 0.0f;
    s_Scroll.y = 0.0f;
  }

  static void ScrollCallback(GLFWwindow *window, double x, double y) {
    (void)window;
    s_Scroll.x = static_cast<float>(x);
    s_Scroll.y = static_cast<float>(y);
  }

  static const glm::vec2 GetScroll() { return s_Scroll; }

  /**
   * @param key Expects a KeyboardKey or MouseButton
   */
  template <typename T> static bool KeyPress(const T &key) {
    if (!s_Window)
      return false;

    int state = 0;

    if constexpr (std::is_same_v<T, KeyboardKey>)
      state = glfwGetKey(s_Window, static_cast<int>(key));

    else if constexpr (std::is_same_v<T, MouseButton>)
      state = glfwGetMouseButton(s_Window, static_cast<int>(key));

    return state == static_cast<int>(KeyAction::PRESS);
  }

  /**
   * @param key Expects a KeyboardKey or MouseButton
   */
  template <typename T> static bool KeyRelease(const T &key) {
    if (!s_Window)
      return false;

    int state = 0;

    if constexpr (std::is_same_v<T, KeyboardKey>)
      state = glfwGetKey(s_Window, static_cast<int>(key));

    else if constexpr (std::is_same_v<T, MouseButton>)
      state = glfwGetMouseButton(s_Window, static_cast<int>(key));

    return state == static_cast<int>(KeyAction::RELEASE);
  }
};

inline glm::vec2 Input::s_Scroll(0.0f);