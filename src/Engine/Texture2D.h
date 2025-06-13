#pragma once

#include <GL/glew.h>
#include <string>

enum class TextureWrap {
  REPEAT = GL_REPEAT,
  MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
  CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
  CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
};

enum class TextureFilter {
  NEAREST = GL_NEAREST,
  LINEAR = GL_LINEAR,
  NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
  LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
  NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
  LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR,
};

class Texture2D {
private:
  unsigned int texture = 0;
  unsigned char *data;
  int width;
  int height;
  GLenum format;

public:
  ~Texture2D();

  void loadFile(const char *filepath);
  void freeFile();
  void setTexture(int level, GLenum internalFormat, GLenum format);
  void setTexture(int level, GLenum internalFormat);
  void setTexture(GLenum internalFormat);
  void setData(unsigned char *data);
  void generateTexture();
  void generateMipmap();
  void setWidth(int width);
  void setHeight(int height);
  void setWrap(TextureWrap s = TextureWrap::REPEAT,
               TextureWrap t = TextureWrap::REPEAT,
               TextureWrap r = TextureWrap::REPEAT) const;
  void setFilter(TextureFilter min = TextureFilter::LINEAR_MIPMAP_LINEAR,
                 TextureFilter mag = TextureFilter::LINEAR) const;
  void setMipmapLevel(int base, int max) const;
  void setBorderColor(float color[4]) const;

  void bind(int activate = -1) const;
  void unbind() const;
};