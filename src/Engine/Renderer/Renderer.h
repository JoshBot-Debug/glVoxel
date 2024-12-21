#pragma once

#include <GL/glew.h>
#include "Engine/Types.h"
#include <iostream>

class Renderer
{
public:
  static void Draw(const DrawElementsIndirectCommand &command, const Primitive &primitive = Primitive::TRIANGLES)
  {
    glDrawElementsIndirect((unsigned int)primitive, GL_UNSIGNED_INT, 0);
  }

  static void Draw(const std::vector<DrawElementsIndirectCommand> &commands, const Primitive &primitive = Primitive::TRIANGLES)
  {
    glMultiDrawElementsIndirect((unsigned int)primitive, GL_UNSIGNED_INT, (const void *)0, commands.size(), 0);
  }
};