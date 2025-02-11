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

App::App() : Window({.title = "glVoxel", .width = 800, .height = 600, .enableDepth = true, .enableVSync = false, .MSAA = 16, .imguiEnableDocking = true, .maximized = true})
{
  controlPanel.setCamera(&camera);
  controlPanel.setResourceManager(&resource);
  controlPanel.setWorld(&world);

  camera.setPosition(4.0f, 3.0f, 8.0f);
  camera.setRotation(-25.0f, 0.0f, 0.0f);
  camera.setProjection(45, 0.01f, 10000.0f);

  Shader &shader = resource.getShader();
  shader.create({
      .name = "default",
      .vertex = "src/Shaders/voxel.vs",
      .fragment = "src/Shaders/voxel.fs",
  });

  uint64_t a = std::bitset<32>(0b10000000000000000000000000000000).to_ullong();

  std::cout << std::bitset<32>((a & ~(1ULL << 31))) << std::endl;
  uint64_t m1 = std::bitset<64>(0b11111111111111111111111111111111).to_ullong();
  uint64_t m2 = std::bitset<64>(0b11111111111111111111111111111111).to_ullong();

  // Shift m1 left and set its LSB to the MSB of m2
  m1 = (m1 << 1) | (((m2 >> 0) & 1ULL) << 33);
  m2 = (m2 << 1) | ((m1 >> 32) & 1ULL);

  std::cout << "m1: " << std::bitset<64>(m1) << std::endl;
  std::cout << "m2: " << std::bitset<64>(m2) << std::endl;
  std::cout << "mx: " << std::bitset<64>() << std::endl;

  // std::thread t([this]()
  //               { BENCHMARK("generateVertexBuffer()", [this]()
  //                           { this->world.update(); }, 1000); });

  // t.join();

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
  shader.bind("default");
  shader.setUniformMatrix4fv("u_View", camera.getViewMatrix());
  shader.setUniformMatrix4fv("u_Projection", camera.getProjectionMatrix());

  shader.setUniform3f("u_CameraPosition", camera.position);

  shader.setUniform3f("u_Material.diffuse", 1.0f, 0.5f, 0.0f);
  shader.setUniform3f("u_Material.specular", 0.5f, 0.5f, 0.5f);
  shader.setUniform1f("u_Material.shininess", 32.0f);

  shader.setUniform3f("u_Light.position", 16.5f, 16.5f, -5.0f);
  shader.setUniform3f("u_Light.specular", 1.0f, 1.0f, 1.0f);
  shader.setUniform3f("u_Light.ambient", 0.2f, 0.2f, 0.2f);
  shader.setUniform3f("u_Light.diffuse", 1.0f, 1.0f, 1.0f);

  world.draw();

  controlPanel.draw();
}

void App::onCleanUp()
{
}