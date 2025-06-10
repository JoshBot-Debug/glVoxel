#include "App.h"
#include "Window/Input.h"
#include "Window/Time.h"

#include "Engine/Camera/PerspectiveCamera.h"
#include "Engine/Shader.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Engine/Types.h"
#include "Debug.h"

App::App() : Window({.title = "glVoxel", .width = 800, .height = 600, .enableDepth = true, .enableVSync = false, .MSAA = 16, .imguiEnableDocking = true, .maximized = true})
{
  controlPanel.setCamera(&camera);
  controlPanel.setResourceManager(&resource);
  controlPanel.setWorld(&world);

  camera.setPosition(-21.0f, 11.0f, -41.0f);
  camera.setRotation(0.0f, 133.0f, 0.0f);
  camera.setProjection(45, 0.01f, 10000.0f);

  const Voxel::SparseVoxelOctree &tree = world.getTree();
  const int size = tree.getSize();

  controlPanel.light.position = {(size / 2), (size / 2) + (size / 4), -(size / 2)};

  Shader &shader = resource.getShader();
  shader.create({
      .name = "voxel",
      .vertex = "src/Shaders/voxel.vs",
      .fragment = "src/Shaders/voxel.fs",
  });
  
  shader.create({
      .name = "skybox",
      .vertex = "src/Shaders/skybox.vs",
      .fragment = "src/Shaders/skybox.fs",
  });

  open();
}

void App::onInitialize()
{
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
}

void App::onUpdate()
{
  const glm::vec2 &size = Window::GetDimensions();

  camera.setViewportSize(size);
  camera.update();

  controlPanel.update();
}

void App::onDraw()
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  Shader &shader = resource.getShader();

  /**
   * Skybox
   */
  skybox.draw(camera, shader, "skybox");

  /**
   * Voxels
   */
  shader.bind("voxel");
  shader.setUniformMatrix4fv("u_View", camera.getViewMatrix());
  shader.setUniformMatrix4fv("u_Projection", camera.getProjectionMatrix());

  shader.setUniform3f("u_CameraPosition", camera.position);

  shader.setUniform3f("u_Material.diffuse", controlPanel.material.diffuse.x, controlPanel.material.diffuse.y, controlPanel.material.diffuse.z);
  shader.setUniform3f("u_Material.specular", controlPanel.material.specular.x, controlPanel.material.specular.y, controlPanel.material.specular.z);
  shader.setUniform1f("u_Material.shininess", controlPanel.material.shininess);

  // Set light properties in the shader
  shader.setUniform3f("u_Light.position", controlPanel.light.position.x, controlPanel.light.position.y, controlPanel.light.position.z);
  shader.setUniform3f("u_Light.specular", controlPanel.light.specular.x, controlPanel.light.specular.y, controlPanel.light.specular.z);
  shader.setUniform3f("u_Light.ambient", controlPanel.light.ambient.x, controlPanel.light.ambient.y, controlPanel.light.ambient.z);
  shader.setUniform3f("u_Light.diffuse", controlPanel.light.diffuse.x, controlPanel.light.diffuse.y, controlPanel.light.diffuse.z);

  world.draw(shader);

  controlPanel.draw();
}

void App::onCleanUp()
{
}