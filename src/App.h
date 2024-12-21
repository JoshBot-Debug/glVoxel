#pragma once

#include <Window/Window.h>

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
  
public:
  App();
  ~App() = default;

  void onUpdate() override;
  void onDraw() override;
  void onCleanUp() override;
};