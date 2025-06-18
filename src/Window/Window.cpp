#include "Window.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Input.h"
#include "Time.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

glm::ivec2 Window::s_Dimensions = glm::ivec2{0, 0};

void errorCallback(int error, const char *description) {
  std::cerr << "GLFW Error " << error << ":" << description << std::endl;
}

void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id,
                              GLenum severity, GLsizei length,
                              const GLchar *message, const void *userParam) {
  (void)length;
  (void)userParam;
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

  glfwShowWindow(m_Window);

  if (m_Options.maximized)
    glfwMaximizeWindow(m_Window);

  while (!glfwWindowShouldClose(m_Window)) {
    if (glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED) != 0) {
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

    glfwSwapBuffers(m_Window);
    Input::ResetScroll();
  }

  this->onCleanUp();
}

const glm::ivec2 &Window::GetDimensions() { return Window::s_Dimensions; }

void Window::setFrameBufferSize(GLFWwindow *window, int w, int h) {
  (void)window;
  glViewport(0, 0, w, h);
  s_Dimensions.x = w;
  s_Dimensions.y = h;
}

Window::Window(const WindowOptions &options) : m_Options(options) {
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

  m_Window = glfwCreateWindow(options.width, options.height,
                              options.title.c_str(), nullptr, nullptr);
  if (!m_Window) {
    glfwTerminate();
    return;
  }

  glfwMakeContextCurrent(m_Window);
  glfwSetFramebufferSizeCallback(m_Window, setFrameBufferSize);
  glfwGetFramebufferSize(m_Window, (int *)&s_Dimensions.x,
                         (int *)&s_Dimensions.y);
  glViewport(0, 0, s_Dimensions.x, s_Dimensions.y);

  const unsigned int init = glewInit();

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
  glfwGetWindowSize(m_Window, &windowWidth, &windowHeight);
  glfwSetWindowPos(m_Window, (screen->width - windowWidth) / 2,
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

  ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  Input::SetWindowContext(m_Window);
  glfwSetScrollCallback(m_Window, Input::ScrollCallback);
}

Window::~Window() {
  glfwDestroyWindow(m_Window);
  glfwTerminate();
}
