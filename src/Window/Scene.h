#pragma once

class Scene {
public:
  virtual ~Scene() = default;

  /**
   * @brief Called to initialize the game or application resources.
   *
   * This method is responsible for setting up necessary components,
   * loading assets, creating game objects, and performing any initial
   * configuration required before the game starts. It should be called
   * at the beginning of the game loop.
   */
  virtual void onInitialize(){};

  /**
   * @brief Called once after onInput() for each frame.
   *
   * This method is responsible for updating the game/application state.
   * It should incorporate changes based on user input and other
   * game logic, such as physics, animations, and AI updates.
   */
  virtual void onUpdate(){};

  /**
   * @brief Called once after onUpdate() for each frame.
   *
   * This method is responsible for rendering the current state of
   * the game/application to the screen. It should clear the
   * previous frame and draw all visual elements based on the
   * updated state.
   */
  virtual void onDraw(){};

  /**
   * @brief Called to clean up resources before the game or application exits.
   *
   * This method is responsible for releasing or unloading any resources
   * that were allocated during initialization, such as textures, sounds,
   * and other game data. It should be called before shutting down the
   * application to prevent memory leaks and ensure proper cleanup.
   */
  virtual void onCleanUp(){};
};