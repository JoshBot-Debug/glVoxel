#pragma once

#include <Window/Window.h>

#include "ECS/Entity.h"

#include "Engine/Camera/PerspectiveCamera.h"
#include "Engine/ResourceManager.h"
#include "Engine/Skybox.h"

#include "World/Raytracer/CPU/ControlPanel.h"
#include "World/Raytracer/CPU/World.h"

#include "Utility.h"

using namespace RaytracerCPU;

const std::string EXE_DIRECTORY = getExecutableDir();

class App : Window {
private:
  World m_World;
  Registry m_Registry;
  PerspectiveCamera m_Camera;
  ResourceManager m_Resource;

  ControlPanel m_ControlPanel;

  Skybox m_Skybox{
      {EXE_DIRECTORY + "/../assets/skybox/plain/left-right-front-back.jpg",
       EXE_DIRECTORY + "/../assets/skybox/plain/left-right-front-back.jpg",
       EXE_DIRECTORY + "/../assets/skybox/plain/top.jpg",
       EXE_DIRECTORY + "/../assets/skybox/plain/bottom.jpg",
       EXE_DIRECTORY + "/../assets/skybox/plain/left-right-front-back.jpg",
       EXE_DIRECTORY + "/../assets/skybox/plain/left-right-front-back.jpg"}};

public:
  App();
  ~App();

  void onInitialize() override;
  void onUpdate() override;
  void onDraw() override;
  void onCleanUp() override;
};