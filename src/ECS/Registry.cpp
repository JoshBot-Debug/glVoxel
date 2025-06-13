#include "Registry.h"
#include "Entity.h"

Registry::~Registry() {
  for (auto entity : this->entt)
    delete entity;
}

Entity *Registry::createEntity(const char *name) {
  ++nEID;
  Entity *entity = new Entity(name, this->nEID, this);
  this->entt.push_back(entity);
  return entity;
}