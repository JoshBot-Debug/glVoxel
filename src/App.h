#pragma once

#include <Window/Window.h>

#include "ECS/Entity.h"

#include "Engine/Camera/PerspectiveCamera.h"
#include "Engine/ResourceManager.h"
#include "Engine/Skybox.h"

#include "EngineControlPanel.h"
#include "World/World.h"

#include "Utility.h"

const std::string EXE_DIRECTORY = getExecutableDir();

class App : Window {
private:
  World world;
  Registry registry;
  PerspectiveCamera camera;
  ResourceManager resource;

  EngineControlPanel controlPanel;

  Skybox skybox{
      {EXE_DIRECTORY + "/../assets/skybox/plain/left-right-front-back.jpg",
       EXE_DIRECTORY + "/../assets/skybox/plain/left-right-front-back.jpg",
       EXE_DIRECTORY + "/../assets/skybox/plain/top.jpg",
       EXE_DIRECTORY + "/../assets/skybox/plain/bottom.jpg",
       EXE_DIRECTORY + "/../assets/skybox/plain/left-right-front-back.jpg",
       EXE_DIRECTORY + "/../assets/skybox/plain/left-right-front-back.jpg"}};

public:
  App();
  ~App() = default;

  void onInitialize() override;
  void onUpdate() override;
  void onDraw() override;
  void onCleanUp() override;
};