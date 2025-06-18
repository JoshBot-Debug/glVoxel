#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "Types.h"

class Model {
private:
  unsigned int m_Id;

  std::vector<Mesh> m_Meshes;

  std::vector<Instance> m_Instances;

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

  unsigned int getID();

  unsigned int createInstance();
  Instance &getInstance(unsigned int id);
  std::vector<Instance> &getInstances();

  const std::vector<Vertex> getVertices() const;
  const std::vector<unsigned int> getIndices() const;
};