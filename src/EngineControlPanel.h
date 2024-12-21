#pragma once

#include <string>
#include <unordered_map>

#include "imgui.h"

#include "Window/Time.h"
#include "Window/Input.h"

#include "Engine/ResourceManager.h"
#include "Engine/Model.h"
#include "Engine/Camera/PerspectiveCamera.h"

class EngineControlPanel
{
private:
  std::vector<Model *> models;
  PerspectiveCamera *camera;
  ResourceManager *resource;

  glm::vec2 mouse;

public:
  std::vector<unsigned int> indices;

  void addModel(Model *model)
  {
    models.push_back(model);
  }

  void setCamera(PerspectiveCamera *camera)
  {
    this->camera = camera;
  }

  void setResourceManager(ResourceManager *resource)
  {
    this->resource = resource;
  }

  void shaderMenu()
  {
    ImGui::Begin("Shaders");

    if (ImGui::Button("Recompile shaders"))
      resource->getShader().recompile();

    ImGui::End();
  }

  void instanceMenu()
  {

    for (const auto &model : resource->getModels())
    {
      std::vector<Instance> &instances = model->getInstances();

      for (size_t i = 0; i < instances.size(); i++)
      {
        Instance &instance = instances[i];

        ImGui::Begin((std::string("Instance: ") + std::to_string(model->getID()) + ":" + std::to_string(i)).c_str());

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

  void cameraMenu()
  {
    ImGui::Begin("Camera");

    ImGui::SeparatorText("Position");

    ImGui::DragFloat("Position X", &camera->position.x, 0.1f);
    ImGui::DragFloat("Position Y", &camera->position.y, 0.1f);
    ImGui::DragFloat("Position Z", &camera->position.z, 0.1f);

    ImGui::SeparatorText("Rotation");

    ImGui::DragFloat("Rotation X", &camera->rotation.x, 0.1f);
    ImGui::DragFloat("Rotation Y", &camera->rotation.y, 0.1f);
    ImGui::DragFloat("Rotation Z", &camera->rotation.z, 0.1f);

    ImGui::SeparatorText("Perspective");

    ImGui::DragFloat("FOV", &camera->fov, 0.1f);
    ImGui::DragFloat("Near", &camera->nearPlane, 0.1f);
    ImGui::DragFloat("Far", &camera->farPlane, 0.1f);

    ImGui::End();
  }

  void update()
  {
    ImGuiIO &io = ImGui::GetIO();

    if (io.WantCaptureKeyboard || io.WantCaptureMouse)
      return;

    float speed = 15.0f;
    double delta = Time::GetDeltaTime();

    glm::vec3 translate(0.0f);

    const glm::vec2 scroll = Input::GetScroll();

    if (scroll.y)
    {
      float direction = (speed * delta * (Input::KeyPress(KeyboardKey::LEFT_CONTROL) ? 10.0f : 50.0f)) * scroll.y;
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

    if (Input::KeyPress(MouseButton::LEFT))
    {
      glm::vec2 dm = glm::mix(glm::vec2(0.0f), Input::MousePosition() - mouse, 0.1f);
      camera->rotate(-dm.y, dm.x, 0.0f);
    }

    camera->translate(translate.x, translate.y, translate.z);

    mouse = Input::MousePosition();
  }

  void stats()
  {
    ImGui::Begin("Stats");
    ImGui::Text("FPS: %i", Time::GetAverageFPS());
    ImGui::End();
  }

  void bufferMenu()
  {
    if (indices.size() == 0)
      return;
    ImGui::Begin("IBO");

    for (size_t i = 0; i < indices.size(); i++)
      ImGui::DragInt(std::to_string(i).c_str(), (int *)&indices[i]);

    ImGui::End();
  }

  void modelsMenu()
  {
    if (models.size() == 0)
      return;

    ImGui::Begin("Models");

    for (Model *model : models)
    {
      ImGui::SeparatorText("ID " + model->getID());

      if (ImGui::Button("Add instance"))
        model->createInstance();
    }
    ImGui::End();
  }

  void draw()
  {
    ImGui::Begin("Debug Menu");

    this->stats();

    this->shaderMenu();

    this->cameraMenu();

    this->instanceMenu();

    this->bufferMenu();

    this->modelsMenu();

    ImGui::End();
  }
};