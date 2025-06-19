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

#include "Voxel/GreedyMeshi256.h"

inline void print_m256i(__m256i val) {
  uint64_t *p = (uint64_t *)&val;
  std::cout << std::bitset<64>(p[0]) << " " << std::bitset<64>(p[1]) << " "
            << std::bitset<64>(p[2]) << " " << std::bitset<64>(p[3]) << "\n"
            << std::endl;
}

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

  m_Camera.setPosition(0.0f, 175.0f, 0.0f);
  m_Camera.setRotation(-20.0f, 130.0f, 0.0f);
  m_Camera.setProjection(45, 0.01f, 10000.0f);

  m_World.setCamera(&m_Camera);
  m_World.setRegistry(&m_Registry);

  m_ControlPanel.light.position = {0, 512, -128};

  Shader &shader = m_Resource.getShader();

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

  // alignas(32) uint64_t raw[4] = {
  //     0b0000000000000000000000000000000000000000000000000000000000000000,
  //     0b0000000000000000000000000000000000000000000000000000000000000000,
  //     0b0000000000000000000000000000000000000000000000000000000000000000,
  //     0b0000000000000000000000000000000000000000000000000000000000000000};

  // __m256i vec = _mm256_load_si256(reinterpret_cast<const __m256i *>(&raw));

  // std::cout << GreedyMeshi256::ctz256(vec) << std::endl;
  // std::cout << _mm256_testz_si256(vec, vec) << std::endl;

  // if (_mm256_testz_si256(vec, vec))
  //   std::cout << "EMPTY" << std::endl;

  // uint64_t mask =
  // 0b0000000000000000000000000000000000000000000000000000000000000000;
  // 0b1111111111111111111111111111111111111111111111111111111111111111;

  // std::cout << std::bitset<64>((mask & ~((1ULL << 63) - 1))) << std::endl;

  // __m256i shifted = _mm256_slli_epi64(a, n);
  // __m256i carry = _mm256_srli_epi64(a, 64 - n);
  // carry = _mm256_permute4x64_epi64(carry, _MM_SHUFFLE(2, 1, 0, 0));
  // carry = _mm256_and_si256(carry, _mm256_set_epi64x(-1ULL, -1ULL, -1ULL,
  // 0ULL));

  // alignas(32) uint64_t raw[4] = {
  //     0b0000111111111111111111111111111111111111111111111111111111111111,
  //     0b1111111111111111111111111111111111111111111111111111111111111111,
  //     0b1111111111111111111111111111111111111111111111111111111111111111,
  //     0b1111111111111111111111111111111111111111111111111111111111111100};

  // alignas(32) uint64_t raw[4] = {
  //     0b1111111111111111111111111111111111111111111111111111111111111111,
  //     0b0111111111111111111111111111111111111111111111111111111111111111,
  //     0b0011111111111111111111111111111111111111111111111111111111111111,
  //     0b0001111111111111111111111111111111111111111111111111111111111111};

  // 0b1111111111111111111111111111111111111111111111111111111111110111
  // 0b1111111111111111111111111111111111111111111111111111111111110011
  // 0b1111111111111111111111111111111111111111111111111111111111110001
  // 0b1111111111111111111111111111111111111111111111111111111111110000

  // union {
  //   __m256i mm_b;
  //   alignas(32) uint64_t b[4];
  // };
  // mm_b = _mm256_load_si256(reinterpret_cast<const __m256i *>(&raw));

  // __m256i left = GreedyMeshi256::sl256(mm_b, 1);

  // union {
  //   __m256i mm_r;
  //   alignas(32) uint64_t r[4];
  // };
  // mm_r = GreedyMeshi256::sr256(mm_b, 1);

  // print_m256i(left);
  // print_m256i(mm_r);

  // int b_clz = GreedyMeshi256::clz256(mm_r);
  // int b_ctz = GreedyMeshi256::ctz256(mm_r);

  // std::cout << b_clz << std::endl;
  // std::cout << b_ctz << std::endl;

  // int c_clz = GreedyMeshi256::clz256(r);
  // int c_ctz = GreedyMeshi256::ctz256(r);

  // std::cout << c_clz << std::endl;
  // std::cout << c_ctz << std::endl;

  // int ffs = GreedyMeshi256::ffs256(mm_r);
  // std::cout << ffs << std::endl;

  // std::cout
  //     << __builtin_ctzll(
  //            0b1111111111111111111111111111111111111111111111111111111111111110)
  //     << std::endl;
  // std::cout
  //     << __builtin_ffsll(
  //            0b1111111111111111111111111111111111111111111111111111111111111110)
  //     << std::endl;

  // int n = 4;
  // uint64_t mask =
  //     0b1111111111111111111111111111111111111111111111111111111111111110;

  // __m256i clb = GreedyMeshi256::clb256(mm_r, 65);
  // print_m256i(clb);
  // std::cout << std::bitset<64>((mask & ~((1ULL << n) - 1))) << std::endl;

  // __m256i full = _mm256_set_epi64x(0ULL,0ULL,0ULL,0ULL);
  // int isFalse = _mm256_testz_si256(clb, clb);
  // int isTrue = _mm256_testz_si256(full, full);

  // std::cout << isFalse << " " << isTrue << std::endl;
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
   * Voxels
   */
  shader.bind("voxel");
  shader.setUniformMatrix4fv("u_View", m_Camera.getViewMatrix());
  shader.setUniformMatrix4fv("u_Projection", m_Camera.getProjectionMatrix());

  shader.setUniform3f("u_CameraPosition", m_Camera.position);

  /**
   * Material (TODO: Remove this)
   */
  shader.setUniform3f("u_Material.diffuse", m_ControlPanel.material.diffuse.x,
                      m_ControlPanel.material.diffuse.y,
                      m_ControlPanel.material.diffuse.z);
  shader.setUniform3f("u_Material.specular", m_ControlPanel.material.specular.x,
                      m_ControlPanel.material.specular.y,
                      m_ControlPanel.material.specular.z);
  shader.setUniform1f("u_Material.shininess",
                      m_ControlPanel.material.shininess);

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