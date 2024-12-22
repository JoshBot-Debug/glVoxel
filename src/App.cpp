#include "App.h"
#include "Window/Input.h"
#include "Window/Time.h"

#include "Engine/Camera/PerspectiveCamera.h"
#include "Engine/Light/PointLight.h"
#include "Engine/Light/DirectionalLight.h"
#include "Engine/Shader.h"
#include "Engine/Texture2D.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Engine/Types.h"
#include "Debug.h"

#include <thread>

const WindowOptions opts = {.title = "glPlay", .width = 800, .height = 600, .enableDepth = true, .enableVSync = false, .MSAA = 16, .imguiEnableDocking = true, .maximized = true};

/**
 * TODO need to add a ViewportManager class that holds all this information\
 * so that it can be edited/controlled from all menus and property panels.
 */
App::App() : Window(opts)
{
  controlPanel.setCamera(&camera);
  controlPanel.setResourceManager(&resource);

  /**
   * Setup a camera
   * Specify the type, and other properties.
   */
  camera.setPosition(0.0f, 0.0f, 20.0f);
  camera.setProjection(45, 0.01f, 10000.0f);

  /**
   * Setup the shader
   */
  Shader &shader = resource.getShader();
  shader.create({
      .name = "default",
      .vertex = "src/Shaders/voxel.vs",
      .fragment = "src/Shaders/voxel.fs",
  });

  /**
   * Load the model foo
   */
  Model *model = resource.loadModel("assets/model/cube.obj");

  world.setModel(model);

  controlPanel.addModel(model);

  std::thread generationThread([this]()
  {
    // BENCHMARK("generateVoxelsWithoutOptimization()", [this]() { this->world.generateVoxelsWithoutOptimization(); }, 1);
    BENCHMARK("generateVoxels()", [this]() { this->world.generateVoxels(); }, 1);
  });

  generationThread.detach();
  
  open();
}

void App::onUpdate()
{
  const glm::vec2 &size = Window::GetDimensions();

  camera.setViewportSize(size);
  camera.update();

  world.update();

  controlPanel.update();
}

void App::onDraw()
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  Shader &shader = resource.getShader();
  shader.bind("default");
  shader.setUniformMatrix4fv("u_View", camera.getViewMatrix());
  shader.setUniformMatrix4fv("u_Projection", camera.getProjectionMatrix());

  shader.setUniform3f("u_CameraPosition", camera.position);

  shader.setUniform3f("u_Material.diffuse", 1.0f, 1.0f, 1.0f);
  shader.setUniform3f("u_Material.specular", 1.0f, 1.0f, 1.0f);
  shader.setUniform1f("u_Material.shininess", 0.4f * 128.0f);

  shader.setUniform3f("u_Light.position", 5.0f, 5.0f, 5.0f);
  shader.setUniform3f("u_Light.specular", 1.0f, 1.0f, 1.0f);
  shader.setUniform3f("u_Light.ambient", 1.0f, 1.0f, 1.0f);
  shader.setUniform3f("u_Light.diffuse", 1.0f, 1.0f, 1.0f);

  glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (const void *)0, world.getInstancesCount());

  controlPanel.draw();
}

void App::onCleanUp()
{
}