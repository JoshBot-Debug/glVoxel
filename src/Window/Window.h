#pragma once

#include <string>
#include <iostream>

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Scene.h"

struct WindowOptions
{
  std::string title;

  unsigned int width;
  unsigned int height;

  bool enableDepth = false;
  bool enableVSync = false;
  int MSAA = 0;

  bool imguiEnableKeyboard = false;
  bool imguiEnableGamepad = false;
  bool imguiEnableDocking = false;

  bool darkMode = false;
  bool maximized = false;
};

class Window : Scene
{
private:
  GLFWwindow *window;

  WindowOptions options;

  static glm::vec2 dimensions;

  static void setFrameBufferSize(GLFWwindow *window, int w, int h);

public:
  Window(const WindowOptions &options);
  ~Window();

  void open();

  static const glm::vec2 &GetDimensions();
};