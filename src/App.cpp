#include "App.h"
#include "Window/Input.h"
#include "Window/Time.h"

#include "Engine/Camera/PerspectiveCamera.h"
#include "Engine/Shader.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Debug.h"
#include "Engine/Types.h"

#include <bitset>

App::App()
    : Window({.title = "glVoxel",
              .width = 800,
              .height = 600,
              .enableDepth = true,
              .enableVSync = true,
              .MSAA = 16,
              .imguiEnableDocking = true,
              .maximized = true}) {
  controlPanel.setCamera(&camera);
  controlPanel.setResourceManager(&resource);
  controlPanel.setWorld(&world);

  camera.setPosition(0.0f, 175.0f, 0.0f);
  camera.setRotation(-20.0f, 130.0f, 0.0f);
  camera.setProjection(45, 0.01f, 10000.0f);

  world.setCamera(&camera);
  world.setRegistry(&registry);

  controlPanel.light.position = {0, 512, -128};

  Shader &shader = resource.getShader();

  shader.create({
      .name = "voxel",
      .vertex = (EXE_DIRECTORY + "/../src/Shaders/voxel.vs").c_str(),
      .fragment = (EXE_DIRECTORY + "/../src/Shaders/voxel.fs").c_str(),
  });

  shader.create({
      .name = "skybox",
      .vertex = (EXE_DIRECTORY + "/../src/Shaders/skybox.vs").c_str(),
      .fragment = (EXE_DIRECTORY + "/../src/Shaders/skybox.fs").c_str(),
  });

  open();

  // uint64_t mask = 0b0000000000000000000000000000000000000000000000000000000000000000;

  // std::cout << std::bitset<64>((mask & ~((1ULL << 63) - 1))) << std::endl;
}

void App::onInitialize() {
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);

  world.initialize();
}

void App::onUpdate() {
  camera.setViewportSize(Window::GetDimensions());
  camera.update();
  world.update();
  controlPanel.update();
}

void App::onDraw() {
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

  /**
   * Material (TODO: Remove this)
   */
  shader.setUniform3f("u_Material.diffuse", controlPanel.material.diffuse.x,
                      controlPanel.material.diffuse.y,
                      controlPanel.material.diffuse.z);
  shader.setUniform3f("u_Material.specular", controlPanel.material.specular.x,
                      controlPanel.material.specular.y,
                      controlPanel.material.specular.z);
  shader.setUniform1f("u_Material.shininess", controlPanel.material.shininess);

  /**
   * Light
   */
  shader.setUniform3f("u_Light.position", controlPanel.light.position.x,
                      controlPanel.light.position.y,
                      controlPanel.light.position.z);
  shader.setUniform3f("u_Light.specular", controlPanel.light.specular.x,
                      controlPanel.light.specular.y,
                      controlPanel.light.specular.z);
  shader.setUniform3f("u_Light.ambient", controlPanel.light.ambient.x,
                      controlPanel.light.ambient.y,
                      controlPanel.light.ambient.z);
  shader.setUniform3f("u_Light.diffuse", controlPanel.light.diffuse.x,
                      controlPanel.light.diffuse.y,
                      controlPanel.light.diffuse.z);

  /**
   * Render the world
   */
  world.draw();

  controlPanel.draw();
}

void App::onCleanUp() {}