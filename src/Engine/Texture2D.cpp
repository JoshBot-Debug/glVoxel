#include "Texture2D.h"
#include "Debug.h"
#include <stb/stb_image.h>

Texture2D::~Texture2D() {
  unbind();
  glDeleteTextures(1, &m_Texture);
  m_Texture = 0;
}

void Texture2D::loadFile(const char *filepath) {
  int nrChannels;
  m_Data = stbi_load(filepath, &m_Width, &m_Height, &nrChannels, 0);
  if (m_Data) {
    if (nrChannels == 1)
      m_Format = GL_RED;
    else if (nrChannels == 3)
      m_Format = GL_RGB;
    else if (nrChannels == 4)
      m_Format = GL_RGBA;
  } else {
    LOG("Texture: Failed to read texture", filepath);
  }
}

void Texture2D::freeFile() { stbi_image_free(m_Data); }

void Texture2D::setTexture(int level, GLenum internalFormat, GLenum format) {
  m_InternalFormat = internalFormat;
  glTexImage2D(GL_TEXTURE_2D, level, internalFormat, m_Width, m_Height, 0,
               format, GL_UNSIGNED_BYTE, m_Data);
}

void Texture2D::setTexture(int level, GLenum internalFormat) {
  m_InternalFormat = internalFormat;
  glTexImage2D(GL_TEXTURE_2D, level, internalFormat, m_Width, m_Height, 0,
               m_Format, GL_UNSIGNED_BYTE, m_Data);
}

void Texture2D::setTexture(GLenum internalFormat) {
  m_InternalFormat = internalFormat;
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, m_Format,
               GL_UNSIGNED_BYTE, m_Data);
}

void Texture2D::setData(unsigned char *data) { m_Data = data; }

void Texture2D::update(unsigned char *data) {
  bind();
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, GL_RGBA,
                  GL_UNSIGNED_BYTE, data);
  unbind();
}

void Texture2D::generate() { glGenTextures(1, &m_Texture); }

void Texture2D::generateMipmap() { glGenerateMipmap(GL_TEXTURE_2D); }

void Texture2D::setWidth(int width) { m_Width = width; }

void Texture2D::setHeight(int height) { m_Height = height; }

void Texture2D::resize(int width, int height) {
  if (m_Width == width || m_Height == height)
    return;
  bind();
  m_Width = width;
  m_Height = height;
  setTexture(m_InternalFormat);
  unbind();
}

void Texture2D::setWrap(TextureWrap s, TextureWrap t, TextureWrap r) const {
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (unsigned int)s);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (unsigned int)t);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, (unsigned int)r);
}

void Texture2D::setFilter(TextureFilter min, TextureFilter mag) const {
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (unsigned int)min);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (unsigned int)mag);
}

void Texture2D::setMipmapLevel(int base, int max) const {
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, base);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, max);
}

void Texture2D::setBorderColor(float color[4]) const {
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
}

void Texture2D::bind(int activate) const {
  if (activate > -1)
    glActiveTexture(GL_TEXTURE0 + activate);
  glBindTexture(GL_TEXTURE_2D, m_Texture);
}

void Texture2D::unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }
