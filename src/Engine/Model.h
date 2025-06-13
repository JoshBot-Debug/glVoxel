#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "Types.h"

class Model {
private:
  unsigned int id;

  std::vector<Mesh> meshes;

  std::vector<Instance> instances;

public:
  Model(unsigned int id, const char *filepath);
  ~Model();

  /**
   * Disable copy constructor
   */
  Model(const Model &) = delete;

  /**
   * Disable assignment operator
   */
  Model &operator=(const Model &) = delete;

  /**
   * Create a move constructor
   */
  Model(Model &&) = default;

  const unsigned int getID() const;

  const unsigned int createInstance();
  Instance &getInstance(unsigned int id);
  std::vector<Instance> &getInstances();

  const std::vector<Vertex> getVertices() const;
  const std::vector<unsigned int> getIndices() const;
};