#pragma once

#include <GL/glew.h>

#include "Engine/Types.h"

class VertexArray
{
private:
  unsigned int vao = 0;

public:
  VertexArray() = default;

  /**
   * Destructor for the VertexArray.
   *
   * This function deletes the OpenGL Vertex Array Object (VAO) when the VertexArray is destroyed.
   * It ensures that the VAO is properly cleaned up, freeing OpenGL resources.
   *
   * If the VAO is valid (not zero), it calls `glDeleteVertexArrays` to delete the VAO from the GPU memory.
   */
  ~VertexArray();

  /**
   * Disable copy constructor
   */
  VertexArray(const VertexArray &) = delete;

  /**
   * Disable assignment operator
   */
  VertexArray &operator=(const VertexArray &) = delete;

  /**
   * Create a move constructor
   */
  VertexArray(VertexArray &&) = default;

  /**
   * Generates the Vertex Array Object (VAO) if it hasn't already been created.
   *
   * This function calls `glGenVertexArrays` to generate a VAO if the `vao` handle is not already initialized.
   */
  void generate();

  /**
   * Sets a vertex attribute pointer for a non-instanced vertex buffer.
   *
   * @param index The index of the vertex attribute (e.g., position, color).
   * @param size The number of components per attribute (e.g., 3 for a 3D vector).
   * @param type The data type of the attribute (e.g., `GL_FLOAT`).
   * @param normalized Specifies whether fixed-point data values should be normalized.
   * @param stride The byte offset between consecutive vertex attributes in the array.
   * @param pointer A pointer to the data in the buffer.
   *
   * This function binds the vertex buffer to the VAO and enables the specified vertex attribute.
   */
  void set(unsigned int index, unsigned int size, VertexType type, bool normalized, size_t stride, const void *pointer) const;

  /**
   * Sets a vertex attribute pointer for an instanced vertex buffer.
   *
   * @param index The index of the vertex attribute (e.g., position, color).
   * @param size The number of components per attribute (e.g., 3 for a 3D vector).
   * @param type The data type of the attribute (e.g., `GL_FLOAT`).
   * @param normalized Specifies whether fixed-point data values should be normalized.
   * @param stride The byte offset between consecutive vertex attributes in the array.
   * @param pointer A pointer to the data in the buffer.
   * @param divisor Specifies the frequency of attribute updates for instancing.
   *
   * This function works similarly to `setVertexAttribPointer`, but it also calls `glVertexAttribDivisor`
   * to control how often the attribute is updated in instanced rendering (used in instanced draws).
   */
  void set(unsigned int index, unsigned int size, VertexType type, bool normalized, size_t stride, const void *pointer, unsigned int divisor) const;

  /**
   * Binds the Vertex Array Object (VAO) to the OpenGL context.
   *
   * This function makes the VAO the active one, meaning subsequent OpenGL calls will use this VAO
   * until another VAO is bound or the binding is unbound.
   */
  void bind() const;

  /**
   * Unbinds the currently bound Vertex Array Object (VAO).
   *
   * This function makes the current VAO no longer active by binding the zero VAO (default VAO).
   */
  void unbind() const;

  /**
   * Gets the OpenGL ID of the Vertex Array Object (VAO).
   *
   * @return The OpenGL ID of the currently bound VAO.
   */
  unsigned int get() const;
};