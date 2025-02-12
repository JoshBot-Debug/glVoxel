#pragma once

#include <string>
#include <unordered_map>

#include "imgui.h"

#include "Window/Time.h"
#include "Window/Input.h"

#include "Engine/ResourceManager.h"
#include "Engine/Model.h"
#include "Engine/Camera/PerspectiveCamera.h"
#include "World/World.h"

struct Material
{
  glm::vec3 diffuse = glm::vec3(0.13f, 0.55f, 0.13f);
  glm::vec3 specular = glm::vec3(0.05f, 0.05f, 0.05f);
  float shininess = 8.0f;
};

struct Light
{
  glm::vec3 position = glm::vec3(16.5f, 16.5f, -5.0f);
  glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
  glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.2f);
  glm::vec3 diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
};

class EngineControlPanel
{
private:
  std::vector<Model *> models;
  PerspectiveCamera *camera;
  ResourceManager *resource;
  World *world;

  glm::vec2 mouse;

public:
  std::vector<unsigned int> indices;

  Light light;
  Material material;

  void addModel(Model *model)
  {
    models.push_back(model);
  }

  void setCamera(PerspectiveCamera *camera)
  {
    this->camera = camera;
  }

  void setWorld(World *world)
  {
    this->world = world;
  }

  void setResourceManager(ResourceManager *resource)
  {
    this->resource = resource;
  }

  void generalMenu()
  {
    ImGui::Begin("General");

    ImGui::SeparatorText("Shaders");

    if (ImGui::Button("Recompile shaders"))
      resource->getShader().recompile();

    ImGui::SeparatorText("Terrain");

    if (ImGui::TreeNode("Draw mode"))
    {
      if (ImGui::Selectable("Draw Traingles", world->drawMode == DrawMode::TRIANGLES))
        world->drawMode = DrawMode::TRIANGLES;

      if (ImGui::Selectable("Draw Lines", world->drawMode == DrawMode::LINES))
        world->drawMode = DrawMode::LINES;
      ImGui::TreePop();
    }

    ImGui::SeparatorText("Mesh Generator");

    ImGui::SeparatorText("Primitives");

    if (ImGui::Button("Generate sphere"))
      world->fillSphere();

    if (ImGui::Button("Generate cube"))
      world->fill();

    ImGui::SeparatorText("Terrain");

    if (ImGui::Button("Generate terrain"))
      world->generateTerrain();

    ImGui::DragInt("Destination map width", &world->terrain.destWidth, 1.0f, Voxel::Chunk::ChunkSize * Voxel::Manager::Chunks);
    ImGui::DragInt("Destination map height", &world->terrain.destHeight, 1.0f, Voxel::Chunk::ChunkSize * Voxel::Manager::Chunks);

    ImGui::DragFloat("Lower X", reinterpret_cast<float *>(&world->terrain.lowerXBound), 0.01f);
    ImGui::DragFloat("Upper X", reinterpret_cast<float *>(&world->terrain.upperXBound), 0.01f);
    ImGui::DragFloat("Lower Z", reinterpret_cast<float *>(&world->terrain.lowerZBound), 0.01f);
    ImGui::DragFloat("Upper Z", reinterpret_cast<float *>(&world->terrain.upperZBound), 0.01f);

    ImGui::SeparatorText("Material");

    ImGui::DragFloat("Diffuse X", &material.diffuse.x, 0.01f, 0.0f);
    ImGui::DragFloat("Diffuse Y", &material.diffuse.y, 0.01f, 0.0f);
    ImGui::DragFloat("Diffuse Z", &material.diffuse.z, 0.01f, 0.0f);

    ImGui::DragFloat("Specular X", &material.specular.x, 0.01f, 0.0f);
    ImGui::DragFloat("Specular Y", &material.specular.y, 0.01f, 0.0f);
    ImGui::DragFloat("Specular Z", &material.specular.z, 0.01f, 0.0f);

    ImGui::DragFloat("Shininess", &material.shininess, 0.1f, 0.0f, 128.0f);

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

  void instanceMenu()
  {
    ImGui::ShowDemoWindow();

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

    this->generalMenu();

    this->cameraMenu();

    this->instanceMenu();

    this->bufferMenu();

    this->modelsMenu();

    ImGui::End();
  }
};