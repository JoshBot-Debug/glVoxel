#pragma once

#include <array>
#include <tuple>

#include "Buffer.h"
#include "VertexArray.h"

/**
 * A triple-buffered wrapper for OpenGL vertex buffer and vertex array objects.
 * Ensures safe and efficient updates by avoiding GPU stalls using sync objects.
 *
 * @tparam bufferTarget The OpenGL buffer target (e.g., GL_ARRAY_BUFFER).
 * @tparam vertexDraw   The draw type (e.g., GL_STATIC_DRAW, GL_DYNAMIC_DRAW).
 */
template <BufferTarget bufferTarget, VertexDraw vertexDraw> class TripleBuffer {
private:
  // Lookup table to get the next buffer index in circular order
  static constexpr uint8_t s_Next[3] = {1, 2, 0};

  // Number of updates queued ahead of the currently rendered buffer
  uint8_t m_Dirty = 0;

  // Index of the buffer currently bound for rendering
  uint8_t m_Current = 0;

  // GPU sync fences for each buffer to track when it's safe to write again
  GLsync m_Fences[3] = {};

  // Triple-buffered vertex buffers
  std::array<Buffer, 3> m_Vbo = {Buffer{bufferTarget, vertexDraw},
                                 Buffer{bufferTarget, vertexDraw},
                                 Buffer{bufferTarget, vertexDraw}};

  // Triple-buffered vertex array objects
  std::array<VertexArray, 3> m_Vao = {VertexArray{}, VertexArray{},
                                      VertexArray{}};

public:
  /**
   * Generates all VAOs and VBOs for the triple buffer.
   */
  void generate() {
    for (size_t i = 0; i < 3; i++) {
      m_Vao[i].generate();
      m_Vbo[i].generate();
    }
  };

  /**
   * Binds the next buffer for rendering.
   * Advances the current buffer index.
   */
  void bind() {
    m_Current = s_Next[m_Current];
    m_Vao[m_Current].bind();
  };

  /**
   * Returns the next available buffer pair (VAO, VBO) for data updates.
   * Increments the dirty counter to track pending updates.
   */
  std::tuple<VertexArray *, Buffer *> get() {
    uint8_t update = s_Next[m_Current];

    if (++m_Dirty == 3)
      m_Dirty = 0;

    return {&m_Vao[update], &m_Vbo[update]};
  }

  /**
   * Checks if the update buffer is ready (not being used by GPU).
   * Uses sync objects to prevent overwriting data still in use.
   *
   * @return true if safe to update, false otherwise.
   */
  bool isDirty() {
    uint8_t update = s_Next[m_Current];

    if (m_Fences[update]) {
      GLenum res =
          glClientWaitSync(m_Fences[update], GL_SYNC_FLUSH_COMMANDS_BIT,
                           // 1ms * 64 (64ms)
                           1'000'000 * 64);
      if (res == GL_TIMEOUT_EXPIRED || res == GL_WAIT_FAILED)
        return false;

      glDeleteSync(m_Fences[update]);
      m_Fences[update] = nullptr;
    }

    return m_Dirty > 0;
  }

  /**
   * Inserts a GPU sync fence for the current buffer.
   * Marks it as in-use so the CPU doesn’t overwrite it prematurely.
   */
  void sync() {
    GLsync &fence = m_Fences[m_Current];
    if (fence)
      glDeleteSync(fence);
    fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  }
};
