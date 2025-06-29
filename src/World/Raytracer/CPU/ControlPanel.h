#pragma once

#include <noise/noise.h>
#include <string>
#include <unordered_map>

#include "imgui.h"

#include "Window/Input.h"
#include "Window/Time.h"

#include "Engine/Camera/PerspectiveCamera.h"
#include "Engine/Model.h"
#include "Engine/ResourceManager.h"
#include "World/Raytracer/CPU/World.h"

namespace RaytracerCPU {

struct Light {
  glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
  glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.2f);
  glm::vec3 diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
};

class ControlPanel {
private:
  std::vector<Model *> m_Models;
  PerspectiveCamera *m_Camera;
  ResourceManager *m_Resource;
  World *m_World;
  Registry *m_Registry;

  glm::vec2 m_Mouse;

public:
  Light light;

  std::vector<unsigned int> indices;

  void addModel(Model *model) { m_Models.push_back(model); }

  void setCamera(PerspectiveCamera *camera) { m_Camera = camera; }

  void setWorld(World *world) { m_World = world; }

  void setRegistry(Registry *registry) { m_Registry = registry; }

  void setResourceManager(ResourceManager *resource) {
    this->m_Resource = resource;
  }

  void generalMenu() {
    ImGui::Begin("General");

    ImGui::SeparatorText("Shaders");

    if (ImGui::Button("Recompile shaders"))
      m_Resource->getShader().recompile();

    if (ImGui::Button("Raytrace"))
      m_Registry->get<CTextureBuffer>()[0]->flush();

    ImGui::SeparatorText("Terrain");

    ImGui::SeparatorText("Mesh Generator");

    ImGui::SeparatorText("Primitives");

    // if (ImGui::Button("Generate sphere"))
    //   m_World->fillSphere();

    // if (ImGui::Button("Generate cube"))
    //   m_World->fill();

    ImGui::SeparatorText("Terrain");

    if (ImGui::Button("Initialize Height Map"))
      m_World->heightMap.initialize();

    ImGui::Spacing();

    ImGui::DragInt("Seed", &m_World->heightMap.terrain.seed);
    ImGui::DragFloat("Scale", &m_World->heightMap.terrain.scale, 0.01f);
    ImGui::DragFloat("Bias", &m_World->heightMap.terrain.bias, 0.01f);

    ImGui::DragFloat("Frequency", &m_World->heightMap.terrain.frequency, 0.01f);
    ImGui::DragFloat("Persistence", &m_World->heightMap.terrain.persistence,
                     0.01f);
    ImGui::DragInt("Octave Count", &m_World->heightMap.terrain.octaveCount,
                   1.0f, 1, noise::module::PERLIN_MAX_OCTAVE);

    ImGui::SeparatorText("Light");

    ImGui::DragFloat("Light Position X", &light.position.x, 0.1f, 0.0f);
    ImGui::DragFloat("Light Position Y", &light.position.y, 0.1f, 0.0f);
    ImGui::DragFloat("Light Position Z", &light.position.z, 0.1f, 0.0f);

    ImGui::DragFloat("Light Specular X", &light.specular.x, 0.01f, 0.0f);
    ImGui::DragFloat("Light Specular Y", &light.specular.y, 0.01f, 0.0f);
    ImGui::DragFloat("Light Specular Z", &light.specular.z, 0.01f, 0.0f);

    ImGui::DragFloat("Light Ambient X", &light.ambient.x, 0.01f, 0.0f);
    ImGui::DragFloat("Light Ambient Y", &light.ambient.y, 0.01f, 0.0f);
    ImGui::DragFloat("Light Ambient Z", &light.ambient.z, 0.01f, 0.0f);

    ImGui::DragFloat("Light Diffuse X", &light.diffuse.x, 0.01f, 0.0f);
    ImGui::DragFloat("Light Diffuse Y", &light.diffuse.y, 0.01f, 0.0f);
    ImGui::DragFloat("Light Diffuse Z", &light.diffuse.z, 0.01f, 0.0f);

    ImGui::End();
  }

  void instanceMenu() {
    ImGui::ShowDemoWindow();

    for (const auto &model : m_Resource->getModels()) {
      std::vector<Instance> &instances = model->getInstances();

      for (size_t i = 0; i < instances.size(); i++) {
        Instance &instance = instances[i];

        ImGui::Begin((std::string("Instance: ") +
                      std::to_string(model->getID()) + ":" + std::to_string(i))
                         .c_str());

        ImGui::SeparatorText("Translate");

        ImGui::DragFloat("Translate X", &instance.translate.x, 0.01f);
        ImGui::DragFloat("Translate Y", &instance.translate.y, 0.01f);
        ImGui::DragFloat("Translate Z", &instance.translate.z, 0.01f);

        ImGui::SeparatorText("Rotation");

        ImGui::DragFloat("Rotation X", &instance.rotation.x, 0.01f);
        ImGui::DragFloat("Rotation Y", &instance.rotation.y, 0.01f);
        ImGui::DragFloat("Rotation Z", &instance.rotation.z, 0.01f);

        ImGui::SeparatorText("Scale");

        ImGui::DragFloat("Scale X", &instance.scale.x, 0.01f);
        ImGui::DragFloat("Scale Y", &instance.scale.y, 0.01f);
        ImGui::DragFloat("Scale Z", &instance.scale.z, 0.01f);

        ImGui::SeparatorText("Color");

        ImGui::DragFloat("Color R", &instance.color.r, 0.1f, 0.0f, 1.0f);
        ImGui::DragFloat("Color G", &instance.color.g, 0.1f, 0.0f, 1.0f);
        ImGui::DragFloat("Color B", &instance.color.b, 0.1f, 0.0f, 1.0f);

        ImGui::End();
      }
    }
  }

  void cameraMenu() {
    ImGui::Begin("Camera");

    ImGui::SeparatorText("Position");

    ImGui::DragFloat("Position X", &m_Camera->position.x, 0.1f);
    ImGui::DragFloat("Position Y", &m_Camera->position.y, 0.1f);
    ImGui::DragFloat("Position Z", &m_Camera->position.z, 0.1f);

    ImGui::SeparatorText("Rotation");

    ImGui::DragFloat("Rotation X", &m_Camera->rotation.x, 0.1f);
    ImGui::DragFloat("Rotation Y", &m_Camera->rotation.y, 0.1f);
    ImGui::DragFloat("Rotation Z", &m_Camera->rotation.z, 0.1f);

    ImGui::SeparatorText("Perspective");

    ImGui::DragFloat("FOV", &m_Camera->fov, 0.1f);
    ImGui::DragFloat("Near", &m_Camera->nearPlane, 0.1f);
    ImGui::DragFloat("Far", &m_Camera->farPlane, 0.1f);

    ImGui::End();
  }

  void update() {
    ImGuiIO &io = ImGui::GetIO();

    if (io.WantCaptureKeyboard || io.WantCaptureMouse)
      return;

    float speed = 30.0f;
    float delta = static_cast<float>(Time::GetDeltaTime());

    glm::vec3 translate(0.0f);

    const glm::vec2 scroll = Input::GetScroll();

    if (scroll.y) {
      float direction =
          (speed * delta *
           (Input::KeyPress(KeyboardKey::LEFT_CONTROL) ? 10.0f : 50.0f)) *
          scroll.y;
      if (Input::KeyPress(KeyboardKey::LEFT_SHIFT))
        translate.x += direction;
      else
        translate.z += direction;
    }

    if (scroll.x)
      translate.x += (speed * delta) * scroll.x * 50.0f;

    if (Input::KeyPress(KeyboardKey::W))
      translate.z += speed * delta;

    if (Input::KeyPress(KeyboardKey::S))
      translate.z -= speed * delta;

    if (Input::KeyPress(KeyboardKey::A))
      translate.x -= speed * delta;

    if (Input::KeyPress(KeyboardKey::D))
      translate.x += speed * delta;

    if (Input::KeyPress(KeyboardKey::E))
      translate.y += speed * delta;

    if (Input::KeyPress(KeyboardKey::Q))
      translate.y -= speed * delta;

    if (Input::KeyPress(MouseButton::LEFT)) {
      glm::vec2 dm =
          glm::mix(glm::vec2(0.0f), Input::MousePosition() - m_Mouse, 0.1f);
      m_Camera->rotate(-dm.y, dm.x, 0.0f);
    }

    m_Camera->translate(translate.x, translate.y, translate.z);

    m_Mouse = Input::MousePosition();
  }

  void stats() {
    ImGui::Begin("Stats");
    ImGui::Text("FPS: %i", Time::GetAverageFPS());
    ImGui::End();
  }

  void bufferMenu() {
    if (indices.size() == 0)
      return;
    ImGui::Begin("IBO");

    for (size_t i = 0; i < indices.size(); i++)
      ImGui::DragInt(std::to_string(i).c_str(), (int *)&indices[i]);

    ImGui::End();
  }

  void m_ModelsMenu() {
    if (m_Models.size() == 0)
      return;

    ImGui::Begin("m_Models");

    for (Model *model : m_Models) {
      ImGui::SeparatorText("ID " + model->getID());

      if (ImGui::Button("Add instance"))
        model->createInstance();
    }
    ImGui::End();
  }

  void draw() {
    ImGui::Begin("Debug Menu");

    this->stats();

    this->generalMenu();

    this->cameraMenu();

    this->instanceMenu();

    this->bufferMenu();

    this->m_ModelsMenu();

    ImGui::End();
  }
};

} // namespace RaytracerCPU
