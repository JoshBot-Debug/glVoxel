#pragma once

#include <Window/Window.h>

#include "Engine/Skybox.h"
#include "Engine/ResourceManager.h"
#include "Engine/Camera/PerspectiveCamera.h"

#include "EngineControlPanel.h"
#include "World/World.h"

class App : Window
{
private:
  ResourceManager resource;
  PerspectiveCamera camera;

  EngineControlPanel controlPanel;
  World world;
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