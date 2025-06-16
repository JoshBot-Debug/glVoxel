#include "Texture2D.h"
#include "Debug.h"
#include <stb/stb_image.h>

Texture2D::~Texture2D() {
  unbind();
  glDeleteTextures(1, &texture);
  texture = 0;
}

void Texture2D::loadFile(const char *filepath) {
  int nrChannels;
  data = stbi_load(filepath, &width, &height, &nrChannels, 0);
  if (data) {
    if (nrChannels == 1)
      format = GL_RED;
    else if (nrChannels == 3)
      format = GL_RGB;
    else if (nrChannels == 4)
      format = GL_RGBA;
  } else {
    LOG("Texture: Failed to read texture", filepath);
  }
}

void Texture2D::freeFile() { stbi_image_free(data); }

void Texture2D::setTexture(int level, GLenum internalFormat, GLenum format) {
  glTexImage2D(GL_TEXTURE_2D, level, internalFormat, width, height, 0, format,
               GL_UNSIGNED_BYTE, data);
}

void Texture2D::setTexture(int level, GLenum internalFormat) {
  glTexImage2D(GL_TEXTURE_2D, level, internalFormat, width, height, 0, format,
               GL_UNSIGNED_BYTE, data);
}

void Texture2D::setTexture(GLenum internalFormat) {
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format,
               GL_UNSIGNED_BYTE, data);
}

void Texture2D::setData(unsigned char *data) { this->data = data; }

void Texture2D::generateTexture() { glGenTextures(1, &texture); }

void Texture2D::generateMipmap() { glGenerateMipmap(GL_TEXTURE_2D); }

void Texture2D::setWidth(int width) { this->width = width; }

void Texture2D::setHeight(int height) { this->height = height; }

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
  glBindTexture(GL_TEXTURE_2D, texture);
}

void Texture2D::unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }
