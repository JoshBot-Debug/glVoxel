#include "App.h"
#include "Window/Input.h"
#include "Window/Time.h"

#include "Engine/Camera/PerspectiveCamera.h"
#include "Engine/Shader.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Debug.h"
#include "Engine/Types.h"

#include <bitset>
#include <immintrin.h>

App::App()
    : Window({.title = "glVoxel",
              .width = 800,
              .height = 600,
              .enableDepth = true,
              .enableVSync = true,
              .MSAA = 16,
              .imguiEnableDocking = true,
              .maximized = true}) {
  m_ControlPanel.setCamera(&m_Camera);
  m_ControlPanel.setResourceManager(&m_Resource);
  m_ControlPanel.setWorld(&m_World);

  m_Camera.setPosition(56.637f, 16.244f, 37.515f);
  m_Camera.setRotation(-20.500f, 0.0f, 0.0f);
  m_Camera.setProjection(45, 0.01f, 10000.0f);

  m_World.setCamera(&m_Camera);
  m_World.setRegistry(&m_Registry);

  m_ControlPanel.light.position = {0, 512, -128};

  Shader &shader = m_Resource.getShader();

  shader.create({
      .name = "image",
      .vertex = (EXE_DIRECTORY + "/../src/Shaders/image.vs").c_str(),
      .fragment = (EXE_DIRECTORY + "/../src/Shaders/image.fs").c_str(),
  });

  shader.create({
      .name = "skybox",
      .vertex = (EXE_DIRECTORY + "/../src/Shaders/skybox.vs").c_str(),
      .fragment = (EXE_DIRECTORY + "/../src/Shaders/skybox.fs").c_str(),
  });

  open();
}

App::~App() {
  for (auto &component : m_Registry.get<CVoxelBuffer>())
    delete component;
}

void App::onInitialize() {
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);

  m_World.initialize();
}

void App::onUpdate() {
  m_Camera.setViewportSize(Window::GetDimensions());
  m_Camera.update();
  m_World.update();
  m_ControlPanel.update();
}

void App::onDraw() {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  Shader &shader = m_Resource.getShader();

  /**
   * Skybox
   */
  m_Skybox.draw(m_Camera, shader, "skybox");

  /**
   * Texture Image Shader
   */
  shader.bind("image");
  shader.setUniformMatrix4fv("u_View", m_Camera.getViewMatrix());
  shader.setUniformMatrix4fv("u_Projection", m_Camera.getProjectionMatrix());
  shader.setUniform3f("u_CameraPosition", m_Camera.position);

  /**
   * Light
   */
  shader.setUniform3f("u_Light.position", m_ControlPanel.light.position.x,
                      m_ControlPanel.light.position.y,
                      m_ControlPanel.light.position.z);
  shader.setUniform3f("u_Light.specular", m_ControlPanel.light.specular.x,
                      m_ControlPanel.light.specular.y,
                      m_ControlPanel.light.specular.z);
  shader.setUniform3f("u_Light.ambient", m_ControlPanel.light.ambient.x,
                      m_ControlPanel.light.ambient.y,
                      m_ControlPanel.light.ambient.z);
  shader.setUniform3f("u_Light.diffuse", m_ControlPanel.light.diffuse.x,
                      m_ControlPanel.light.diffuse.y,
                      m_ControlPanel.light.diffuse.z);

  /**
   * Render the m_World
   */
  m_World.draw();

  m_ControlPanel.draw();
}

void App::onCleanUp() {}