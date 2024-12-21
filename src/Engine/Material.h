#pragma once

#include <glm/glm.hpp>

#include "Texture2D.h"

// Albedo Map - texture with no shadows
// Metalness - 0 - 1
// Metal map - texture black/white no metal/metal areas
// Roughness map - black/white texture
// Specular map - black/white texture
// Anistrophic map - rarely used
// Normal Map
// Bump map - grayscale texture
// Displacement map - heavy on the gpu
// Opacity
// Opacity map
// Subsurface scattering
// Emission
// Ambient Occlusion

class Material
{
private:
  unsigned int id;

public:
  Material(unsigned int id);
  ~Material() = default;

  void setDiffuseTexture(const Texture2D *texture);
  void setDiffuseColor(const glm::vec3 &color);
  void setRoughness(const float &value);
  void setShininess(const float &value);
};