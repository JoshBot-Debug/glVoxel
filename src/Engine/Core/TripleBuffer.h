#pragma once

#include <array>
#include <tuple>

#include "Buffer.h"
#include "VertexArray.h"

template <BufferTarget bufferTarget, VertexDraw vertexDraw> class TripleBuffer {
private:
  static constexpr uint8_t previous[3] = {2, 0, 1};
  static constexpr uint8_t next[3] = {1, 2, 0};

  uint8_t _dirty = 0;
  uint8_t current = 0;

  GLsync fences[3] = {};

  std::array<Buffer, 3> vbo = {Buffer{bufferTarget, vertexDraw},
                               Buffer{bufferTarget, vertexDraw},
                               Buffer{bufferTarget, vertexDraw}};

  std::array<VertexArray, 3> vao = {VertexArray{}, VertexArray{},
                                    VertexArray{}};

public:
  void generate() {
    for (size_t i = 0; i < 3; i++) {
      vao[i].generate();
      vbo[i].generate();
    }
  };

  void bind() {
    current = next[current];
    vao[current].bind();
  };

  std::tuple<VertexArray *, Buffer *> get() {
    uint8_t update = next[current];

    if (++_dirty == 3)
      _dirty = 0;

    return {&vao[update], &vbo[update]};
  }

  bool isDirty() {
    uint8_t update = next[current];

    if (fences[update]) {
      GLenum res = glClientWaitSync(fences[update], GL_SYNC_FLUSH_COMMANDS_BIT,
                                    // 1ms * 64 (64ms)
                                    1'000'000 * 64);
      if (res == GL_TIMEOUT_EXPIRED || res == GL_WAIT_FAILED)
        return false;

      glDeleteSync(fences[update]);
      fences[update] = nullptr;
    }

    return _dirty > 0;
  }

  void sync() {
    GLsync &fence = fences[current];
    if (fence)
      glDeleteSync(fence);
    fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  }
};
