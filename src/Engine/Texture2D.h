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
  unsigned int m_Texture = 0;
  unsigned char *m_Data;
  int m_Width;
  int m_Height;
  GLenum m_Format;

public:
  /**
   * Destructor: unbinds and deletes the OpenGL texture.
   */
  ~Texture2D();

  /**
   * Loads image data from file using stb_image.
   * NOTE: You must call freeFile() if you use loadFile()
   */
  void loadFile(const char *filepath);

  /**
   * Frees image data loaded by stb_image.
   */
  void freeFile();

  /**
   * Uploads texture data with explicit level, internal format, and format.
   * NOTE: You must call setData() before calling setTexture()
   */
  void setTexture(int level, GLenum internalFormat, GLenum format);

  /**
   * Uploads texture data with explicit level and internal format (uses stored
   * format).
   */
  void setTexture(int level, GLenum internalFormat);

  /**
   * Uploads texture data using default mipmap level (0) and given internal
   * format.
   */
  void setTexture(GLenum internalFormat);

  /**
   * Sets the texture pixel data directly.
   */
  void setData(unsigned char *data);

  /**
   * Generates an OpenGL texture object.
   */
  void generateTexture();

  /**
   * Generates mipmaps for the currently bound texture.
   */
  void generateMipmap();

  /**
   * Sets the texture width.
   */
  void setWidth(int width);

  /**
   * Sets the texture height.
   */
  void setHeight(int height);

  /**
   * Sets texture wrapping modes for S, T, and R axes.
   */
  void setWrap(TextureWrap s = TextureWrap::REPEAT,
               TextureWrap t = TextureWrap::REPEAT,
               TextureWrap r = TextureWrap::REPEAT) const;

  /**
   * Sets texture filtering modes for minification and magnification.
   */
  void setFilter(TextureFilter min = TextureFilter::LINEAR_MIPMAP_LINEAR,
                 TextureFilter mag = TextureFilter::LINEAR) const;

  /**
   * Sets the base and max mipmap levels for this texture.
   */
  void setMipmapLevel(int base, int max) const;

  /**
   * Sets the texture border color (used when wrap mode is GL_CLAMP_TO_BORDER).
   */
  void setBorderColor(float color[4]) const;

  /**
   * Binds the texture to the given texture unit (default is active texture).
   */
  void bind(int activate = -1) const;

  /**
   * Unbinds any texture bound to GL_TEXTURE_2D.
   */
  void unbind() const;
};