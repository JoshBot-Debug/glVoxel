#pragma once

#include <Window/Window.h>

#include "ECS/Entity.h"

#include "Engine/Camera/PerspectiveCamera.h"
#include "Engine/ResourceManager.h"
#include "Engine/Skybox.h"

#include "EngineControlPanel.h"
#include "World/World.h"

class App : Window {
private:
  World world;
  Registry registry;
  PerspectiveCamera camera;
  ResourceManager resource;

  EngineControlPanel controlPanel;

  Skybox skybox{{"assets/skybox/plain/left-right-front-back.jpg",
                 "assets/skybox/plain/left-right-front-back.jpg",
                 "assets/skybox/plain/top.jpg",
                 "assets/skybox/plain/bottom.jpg",
                 "assets/skybox/plain/left-right-front-back.jpg",
                 "assets/skybox/plain/left-right-front-back.jpg"}};

public:
  App();
  ~App() = default;

  void onInitialize() override;
  void onUpdate() override;
  void onDraw() override;
  void onCleanUp() override;
};