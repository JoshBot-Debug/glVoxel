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

  // t.detach();

  // std::bitset<64> bits("1111111111111111111111111111111111111111111111111111111111111111");
  // std::bitset<64> bits("1111111111111111111111111111111111111111111111111111111111111110");
  // std::bitset<64> bits("1111111111111111111111111111111111111111111111111111111111111111");
  // std::bitset<64> bits("0111111111111111111111111111111111111111111111111111111111111111");
  // std::bitset<64> bits("1111111111111111111111111111111100000000000000000000000000000000");
  // int offset = __builtin_ffsll(bits.to_ullong());

  // if (offset)
  //   bits = bits >> offset - 1;

  // int size = __builtin_ctzll(~bits.to_ullong());

  // std::cout << offset << std::endl;
  // std::cout << size << std::endl;

  // std::thread t([this]() mutable
  //               { BENCHMARK("generateVertexBuffer()", [this]() mutable
  //                           { std::vector<Vertex> vertices;
  //                             this->world.generateMesh(vertices); }, 1000); });

  // t.detach();

  std::vector<Vertex> vertices;
  this->world.generateMesh(vertices);
  this->world.setBuffer(vertices);

  open();
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

  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);

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

  glDrawArrays(GL_LINES, 0, 147456);
  // glDrawArrays(GL_TRIANGLES, 0, 147456);
  // glDrawElements(GL_TRIANGLES, 38460, GL_UNSIGNED_INT, 0);
  // glDrawElements(GL_LINES, 38460, GL_UNSIGNED_INT, 0);

  // glDrawArrays(GL_TRIANGLES, 0, 49152);
  // glDrawArrays(GL_LINES, 0, 49152);

  // glDrawArrays(GL_TRIANGLES, 0, 8);
  // glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

  controlPanel.draw();
}

void App::onCleanUp()
{
}