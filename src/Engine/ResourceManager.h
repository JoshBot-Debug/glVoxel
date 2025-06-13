#pragma once

#include <string>
#include <unordered_map>

#include "Model.h"
#include "Shader.h"

class ResourceManager {
private:
  Shader shader;
  std::vector<Model *> models;

public:
  ResourceManager() = default;

  ResourceManager(const ResourceManager &) = delete;

  ~ResourceManager() {
    for (const auto &model : models)
      delete model;
  };

  /**
   * @returns A pointer to a Model
   */
  Model *loadModel(const char *path) {
    unsigned int id = models.size();

    Model *model = new Model(id, path);
    models.push_back(model);

    return model;
  }

  /**
   * Get the Model by id
   * @returns The Model
   */
  Model *getModel(unsigned int id) { return models.at(id); }

  /**
   * Gets all models
   * @returns A vector of Model pointers
   */
  const std::vector<Model *> &getModels() const { return models; }

  Shader &getShader() { return shader; };
};