#pragma once

#include <unordered_map>

#include "Engine/Types.h"
#include "Engine/Model.h"
#include "Engine/Core/Buffer.h"

struct DrawElementsIndirectMesh
{
  Model *model;
  unsigned int partitionID;
  unsigned int command;
  std::unordered_map<unsigned int, unsigned int> instanceIDToOffset;
};

class DrawElementsIndirect
{
private:
  Buffer indirect;
  MultiModelInstanceBuffer buffer;
  std::vector<DrawElementsIndirectCommand> commands;
  std::unordered_map<unsigned int, DrawElementsIndirectMesh> meshes;

public:
  DrawElementsIndirect() : indirect(BufferTarget::DRAW_INDIRECT_BUFFER, VertexDraw::STATIC)
  {
    indirect.generate();
  };

  DrawElementsIndirect(unsigned int vboSize, unsigned int eboSize) : indirect(BufferTarget::DRAW_INDIRECT_BUFFER, VertexDraw::STATIC)
  {
    indirect.generate();
    buffer.resize(vboSize, eboSize);
  };

  /**
   * Disable copy constructor
   */
  DrawElementsIndirect(const DrawElementsIndirect &) = delete;

  /**
   * Disable assignment operator
   */
  DrawElementsIndirect &operator=(const DrawElementsIndirect &) = delete;

  /**
   * Create a move constructor
   */
  DrawElementsIndirect(DrawElementsIndirect &&) = default;

  /**
   * Takes in a model, adds it to the buffer and returns an ID that can be used to reference it.
   * @param model The model, this class will not own the pointer. You must free it once you're done using it.
   */
  void addModel(Model *model)
  {
    DrawElementsIndirectMesh &mesh = meshes[model->getID()];
    mesh.model = model;
    mesh.command = commands.size();
    DrawElementsIndirectCommand &command = commands.emplace_back();
    mesh.partitionID = buffer.addBufferData(model->getVertices(), model->getIndices(), command.firstIndex, command.baseVertex);
  }

  void update()
  {
    for (auto &[modelID, mesh] : meshes)
    {
      const std::vector<Instance> &instances = mesh.model->getInstances();
      DrawElementsIndirectCommand &command = commands[mesh.command];

      const std::vector<unsigned int> instanceIDs = buffer.add(mesh.partitionID, instances, command.baseInstance);
      command.count = mesh.model->getIndices().size();
      command.primCount = instances.size();

      for (size_t i = 0; i < instances.size(); i++)
        mesh.instanceIDToOffset[instances[i].id] = instanceIDs[i];
    }

    indirect.set(commands);
  }

  void update(unsigned int modelID, unsigned int instanceID, Instance &instance)
  {
    buffer.update(meshes[modelID].partitionID, meshes[modelID].instanceIDToOffset[instanceID], instance);
  }

  void update(unsigned int modelID, unsigned int offsetInstanceID, std::vector<Instance> &instance)
  {
    buffer.update(meshes[modelID].partitionID, meshes[modelID].instanceIDToOffset[offsetInstanceID], instance);
  }

  DrawElementsIndirectCommand &getCommand(unsigned int modelID)
  {
    return commands[meshes[modelID].command];
  }

  std::vector<DrawElementsIndirectCommand> getCommands()
  {
    return commands;
  }

  void bind() {
    buffer.bind();
    indirect.bind();
  }

  void unbind() {
    buffer.unbind();
    indirect.unbind();
  }
};