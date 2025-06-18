#pragma once

#include <iostream>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Scene.h"

struct WindowOptions {
  std::string title;

  int width;
  int height;

  bool enableDepth = false;
  bool enableVSync = false;

  int MSAA = 0;

  bool imguiEnableKeyboard = false;
  bool imguiEnableGamepad = false;
  bool imguiEnableDocking = false;

  bool darkMode = false;
  bool maximized = false;
};

class Window : Scene {
private:
  GLFWwindow *m_Window;

  WindowOptions m_Options;

  static glm::ivec2 s_Dimensions;

  static void setFrameBufferSize(GLFWwindow *window, int w, int h);

public:
  Window(const WindowOptions &options);
  ~Window();

  void open();

  static const glm::ivec2 &GetDimensions();
};