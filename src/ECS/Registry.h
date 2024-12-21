#pragma once

#include <any>
#include <vector>
#include <unordered_map>

// This is called all so it's easy to undersand when looking at the
// intellisense for methods, basically it's the initial value which is zero.
// all ids start from 1
const int ALL = 0;

class Entity;

using EntityID = int;

/**
 * Registry is a container for managing entities and their associated components.
 * It supports adding, retrieving, and deleting components of various types
 * associated with each entity.
 */
class Registry
{
private:
  EntityID nEID = ALL;                                         ///< The next available entity ID.
  std::vector<Entity *> entt;                                  ///< List of all entities in the registry.
  std::unordered_map<EntityID, std::vector<std::any>> storage; ///< Storage of components indexed by entity ID.

public:
  Registry() = default;
  ~Registry();

  /**
   * Creates a new entity with a given name.
   *
   * @param name The name of the entity.
   * @return A pointer to the newly created entity.
   */
  Entity *createEntity(const char *name);

  /**
   * Adds a component of type T to the specified entity.
   *
   * @param entity The entity ID to which the component will be added.
   * @param args Constructor arguments for the component.
   * @return A pointer to the newly created component.
   */
  template <typename T, typename... Args>
  T *add(EntityID entity, Args &&...args)
  {
    T *component = new T(std::forward<Args>(args)...);
    this->storage[entity].push_back((std::any)component);
    return component;
  }

  /**
   * Checks if an entity has a specific component type.
   *
   * @param entity The entity ID to check.
   * @return True if the entity has the component, false otherwise.
   */
  template <typename T>
  bool has(EntityID entity)
  {
    return (this->storage.find(entity) != this->storage.end());
  }

  /**
   * Collects components of specified types from a single entity.
   *
   * @param entity The entity ID from which to collect components.
   * @return A tuple containing pointers to the components.
   */
  template <typename... T>
  std::tuple<T *...> collect(EntityID entity)
  {
    return std::make_tuple(get<T>(entity)...);
  }

  /**
   * Retrieves a component of type T from a specified entity.
   *
   * @param entity The entity ID from which to retrieve the component.
   * @return A pointer to the component, or nullptr if not found.
   */
  template <typename T>
  T *get(EntityID entity)
  {
    for (auto &component : this->storage[entity])
      try
      {
        return std::any_cast<T *>(component);
      }
      catch (const std::bad_any_cast &e)
      {
      }
    return nullptr;
  }

  /**
   * Collects all components of specified types across all entities.
   *
   * @return A tuple containing vectors of pointers to the components.
   */
  template <typename... T>
  std::tuple<std::vector<T *>...> collect()
  {
    return std::make_tuple(get<T>()...);
  }

  /**
   * Retrieves all components of a specified type across all entities.
   *
   * @return A vector of pointers to the components.
   */
  template <typename T>
  std::vector<T *> get()
  {
    std::vector<T *> result;

    for (const auto &[eid, components] : this->storage)
      for (auto &component : components)
        try
        {
          result.push_back(std::any_cast<T *>(component));
        }
        catch (const std::bad_any_cast &e)
        {
        }

    return result;
  }

  /**
   * Retrieves all entities in the registry.
   *
   * @return A vector of pointers to all entities.
   */
  std::vector<Entity *> entities()
  {
    return this->entt;
  }

  /**
   * Frees a specific component type from the specified entity.
   *
   * @param entity The entity ID from which to free the component.
   */
  template <typename T>
  void free(EntityID entity)
  {
    if (this->storage.find(entity) == this->storage.end())
      return;

    for (const auto &component : this->storage[entity])
    {
      try
      {
        delete std::any_cast<T *>(component);
      }
      catch (const std::bad_any_cast &e)
      {
        // The cast failed, this is no the object we want, skip.
      }
    }
  }

  /**
   * Frees all components of a specified type across all entities.
   */
  template <typename... T>
  void free()
  {
    for (const auto [eid, components] : this->storage)
      (this->free<T>(eid), ...);
  }
};