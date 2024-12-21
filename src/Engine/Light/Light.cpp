#include "Light.h"

Light::Light(unsigned int id): id(id)
{
}

const unsigned int Light::getID() const
{
  return id;
}