#pragma once

#include "Registry.h"
#include <string>

using EntityID = int;

/**
 * Entity represents an object within the ECS.
 * It holds a unique identifier, a name, and a reference to the Registry
 * that manages its components.
 */
class Entity {
private:
  EntityID id;        ///< Unique identifier for the entity.
  std::string name;   ///< Name of the entity.
  Registry *registry; ///< Pointer to the Registry managing this entity.

public:
  /**
   * Constructs an Entity with a given name, ID, and registry reference.
   *
   * @param name The name of the entity.
   * @param id The unique identifier for the entity.
   * @param registry A pointer to the Registry managing this entity.
   */
  Entity(const std::string &name, int id, Registry *registry)
      : id(id), name(name), registry(registry){};

  /**
   * Retrieves the entity's unique identifier.
   *
   * @return The entity's ID.
   */
  EntityID getId() { return this->id; }

  /**
   * Adds a component of type T to this entity.
   *
   * @param args Constructor arguments for the component.
   * @return A pointer to the newly created component.
   */
  template <typename T, typename... Args> T *add(Args &&...args) {
    return this->registry->add<T>(this->id, std::forward<Args>(args)...);
  }

  /**
   * Checks if this entity has a component of type T.
   *
   * @return True if the entity has the component, false otherwise.
   */
  template <typename T> bool has() { return this->registry->has<T>(this->id); }

  /**
   * Collects components of specified types from this entity.
   *
   * @return A tuple containing pointers to the collected components.
   */
  template <typename... T> std::tuple<T *...> collect() {
    return this->registry->collect<T...>(this->id);
  }

  /**
   * Retrieves a component of type T from this entity.
   *
   * @return A pointer to the component, or nullptr if not found.
   */
  template <typename T> T *get() { return this->registry->get<T>(this->id); }

  /**
   * Compares the name of this entity with a given name.
   *
   * @param name The name to compare against.
   * @return True if the names match, false otherwise.
   */
  bool is(const std::string &name) { return this->name == name; }

  /**
   * Frees components of specified types from this entity
   * or all components for the entity if none are specified.
   *
   * This method also deletes the entity itself.
   */
  template <typename... T> void free() {
    (this->registry->free<T>(this->id), ...);
    delete this;
  }

  /**
   * Equality operator to compare two entities by their IDs.
   *
   * @param other The other entity to compare.
   * @return True if both entities have the same ID, false otherwise.
   */
  bool operator==(const Entity &other) const { return id == other.id; };

  /**
   * Implicit conversion operator to convert Entity to EntityID.
   *
   * @return The entity's ID.
   */
  operator EntityID() const { return this->id; }
};