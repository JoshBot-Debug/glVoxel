#pragma once

#include <string>
#include <GL/glew.h>

enum class TextureWrap
{
  REPEAT = GL_REPEAT,
  MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
  CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
  CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
};

enum class TextureFilter
{
  NEAREST = GL_NEAREST,
  LINEAR = GL_LINEAR,
  NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
  LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
  NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
  LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR,
};

class Texture2D
{
private:
  unsigned int texture = 0;

public:
  Texture2D(const char *filepath);
  ~Texture2D();

  void setWrap(TextureWrap s = TextureWrap::REPEAT, TextureWrap t = TextureWrap::REPEAT, TextureWrap r = TextureWrap::REPEAT) const;
  void setFilter(TextureFilter min = TextureFilter::LINEAR_MIPMAP_LINEAR, TextureFilter mag = TextureFilter::LINEAR) const;
  void setMipmapLevel(int base, int max) const;
  void setBorderColor(float color[4]) const;

  void bind(int activate = -1) const;
  void unbind() const;
};