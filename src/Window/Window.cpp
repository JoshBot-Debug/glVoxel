#include "Window.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Input.h"
#include "Time.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

glm::vec2 Window::dimensions = glm::vec2(0.0f, 0.0f);

void errorCallback(int error, const char *description) {
  std::cerr << "GLFW Error " << error << ":" << description << std::endl;
}

void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id,
                              GLenum severity, GLsizei length,
                              const GLchar *message, const void *userParam) {
  std::cerr << "OpenGL Debug Message (" << id << "): " << message << std::endl;
  std::cerr << "Source: " << source << ", Type: " << type
            << ", Severity: " << severity << std::endl;
}

static void glDebug() {
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

  glDebugMessageCallback(debugCallback, nullptr);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr,
                        GL_TRUE);
}

void Window::open() {
  this->onInitialize();

  glfwShowWindow(window);

  if (options.maximized)
    glfwMaximizeWindow(window);

  while (!glfwWindowShouldClose(window)) {
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }

    Time::UpdateDeltaTime();

    glfwPollEvents();

    this->onUpdate();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    this->onDraw();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    Input::ResetScroll();
  }

  this->onCleanUp();
}

const glm::vec2 &Window::GetDimensions() { return Window::dimensions; }

void Window::setFrameBufferSize(GLFWwindow *window, int w, int h) {
  glViewport(0, 0, w, h);
  dimensions.x = w;
  dimensions.y = h;
}

Window::Window(const WindowOptions &options) : options(options) {
  glfwSetErrorCallback(errorCallback);

  if (!glfwInit())
    std::cerr << "GLFW initialization failed!" << std::endl;

  const char *glsl_version = "#version 330 core";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  glfwWindowHint(GLFW_STENCIL_BITS, 8);
  glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

  if (options.MSAA)
    glfwWindowHint(GLFW_SAMPLES, options.MSAA);

  window = glfwCreateWindow(options.width, options.height,
                            options.title.c_str(), nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return;
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, setFrameBufferSize);
  glfwGetFramebufferSize(window, (int *)&dimensions.x, (int *)&dimensions.y);
  glViewport(0, 0, dimensions.x, dimensions.y);

  const int init = glewInit();

  if (init != GLEW_OK)
    std::cerr << "GLEW initialization failed! Error code: " << init
              << std::endl;

  std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

  glDebug();

  glfwSwapInterval(options.enableVSync ? 1 : 0);

  if (options.enableDepth)
    glEnable(GL_DEPTH_TEST);

  const GLFWvidmode *screen = glfwGetVideoMode(glfwGetPrimaryMonitor());
  int windowWidth, windowHeight;
  glfwGetWindowSize(window, &windowWidth, &windowHeight);
  glfwSetWindowPos(window, (screen->width - windowWidth) / 2,
                   (screen->height - windowHeight) / 2);

  IMGUI_CHECKVERSION();

  ImGui::CreateContext();

  ImGuiIO &io = ImGui::GetIO();

  if (options.imguiEnableKeyboard)
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  if (options.imguiEnableGamepad)
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  if (options.imguiEnableDocking)
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  if (options.darkMode)
    ImGui::StyleColorsDark();
  else
    ImGui::StyleColorsLight();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  Input::SetWindowContext(window);
  glfwSetScrollCallback(window, Input::ScrollCallback);
}

Window::~Window() {
  glfwDestroyWindow(window);
  glfwTerminate();
}
