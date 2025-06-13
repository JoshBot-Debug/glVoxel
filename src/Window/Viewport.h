#pragma once

#include <glm/glm.hpp>

#include "Scene.h"

class Viewport : public Scene {
private:
  unsigned int texture = 0;
  unsigned int framebuffer = 0;

  const char *title = "Viewport";

  // Viewport width and height
  glm::vec2 dimensions{0, 0};

  // Viewport position
  glm::vec2 position{0, 0};

  /**
   * Resizes the viewport and updates the texture accordingly.
   *
   * @param size This new width and height of the viewport
   */
  void resize(glm::vec2 size);

  /**
   * Create the framebuffer and the texture to render to.
   */
  void createFrameBuffer();

public:
  Viewport() = default;

  virtual ~Viewport();

  /**
   * @brief Sets the title of the window.
   *
   * This method updates the window title to the specified string.
   *
   * @param title A pointer to a null-terminated character string that
   *              represents the new title for the window.
   *              It should not be modified after this call.
   */
  void setTitle(const char *title);

  /**
   * Sets the dimensions of the viewport.
   *
   * NOTE: This will not update a texture that has already been created
   *       for that, you have to call resize.
   *
   * @param size This new width and height of the viewport
   */
  void setDimensions(glm::vec2 size);

  /**
   * Do not override this method, the implimentation is done in the viewport.
   * Instead use onDrawViewport()
   */
  void onDraw() final;

  /**
   * Use this in place of onDraw();
   */
  virtual void onDrawViewport(){};

  /**
   * Get the size of the viewport
   */
  glm::vec2 *getDimensions();

  /**
   * Get the position of the viewport
   */
  glm::vec2 *getPosition();

  /**
   * Called when the viewport is resized.
   */
  virtual void onResize(glm::vec2 size){};
};