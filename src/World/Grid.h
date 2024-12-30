#pragma once

#include "Engine/Core/Buffer.h"
#include "Engine/Core/VertexArray.h"
#include "Engine/Model.h"
#include "World/Voxel.h"
#include "World/Octree.h"
#include "World/UniformGrid3D.h"

const std::vector<glm::vec3> CubeVertices = {
    // Bottom face
    {0.0f, 0.0f, 0.0f},
    {1.0f, 0.0f, 0.0f},

    {1.0f, 0.0f, 0.0f},
    {1.0f, 0.0f, 1.0f},

    {1.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 1.0f},

    {0.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 0.0f},

    // Top face
    {0.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 0.0f},

    {1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f},

    {1.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 1.0f},

    {0.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 0.0f},

    // Vertical edges
    {0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},

    {1.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 0.0f},

    {1.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 1.0f},

    {0.0f, 0.0f, 1.0f},
    {0.0f, 1.0f, 1.0f},
};

class Grid
{
private:
  VertexArray vao;
  Buffer vbo;

public:
  Grid() : vbo(BufferTarget::ARRAY_BUFFER)
  {
    vao.generate();
    vbo.generate();

    vao.bind();
    vbo.set(CubeVertices);
    vao.set(0, 3, VertexType::FLOAT, false, 0, (const void *)0);
  }

  void draw()
  {
    vao.bind();
    glDrawArrays(GL_LINES, 0, CubeVertices.size());
    glBindVertexArray(0);
  }
};