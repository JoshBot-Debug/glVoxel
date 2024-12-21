#pragma once

#include "Engine/Types.h"
#include "Engine/Core/VertexArray.h"
#include "Engine/Core/Buffer.h"

struct Instance;

class MultiModelInstanceBuffer
{
private:
  VertexArray vao;
  Buffer vbo;
  Buffer ebo;
  Buffer ibo;

public:
  MultiModelInstanceBuffer();

  /**
   * Disable copy constructor
   */
  MultiModelInstanceBuffer(const MultiModelInstanceBuffer &) = delete;

  /**
   * Disable assignment operator
   */
  MultiModelInstanceBuffer &operator=(const MultiModelInstanceBuffer &) = delete;

  /**
   * Create a move constructor
   */
  MultiModelInstanceBuffer(MultiModelInstanceBuffer &&) = default;

  /**
   * @param vertices The virtex data
   * @param indices The indice data
   *
   * @param firstIndex First index of the indice in the ebo.
   * @param baseVertex First index of the vertices in the vbo.
   *
   * @returns The partition ID which you can use to create instances of this mesh
   */
  const unsigned int addBufferData(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, unsigned int &firstIndex, unsigned int &baseVertex);

  /**
   * @param partition The partition we are creating or updating. Starts from 0, MUST GO IN SEQUENCE
   * @param instances The instance data
   *
   * @param baseInstance First index of the instance data in the instance vbo.
   *
   * @returns The offset of the instance
   */
  unsigned int add(const unsigned int partition, const Instance &instance, unsigned int &baseInstance);

  /**
   * @param partition The partition we are creating or updating. Starts from 0, MUST GO IN SEQUENCE
   * @param offset The offset in the partiton where you want to insert or update
   * @param instances The instance data
   *
   * @param baseInstance First index of the instance data in the instance vbo.
   *
   * @returns The offsets of the instances
   */
  std::vector<unsigned int> add(const unsigned int partition, const std::vector<Instance> &instances, unsigned int &baseInstance);

  /**
   * @param partition The partition we are creating or updating. Starts from 0, MUST GO IN SEQUENCE
   * @param offset The offset in the partiton where you want to insert or update
   * @param instance The instance data
   */
  void update(const unsigned int partition, const unsigned int offset, Instance &instance);

  /**
   * @param partition The partition we are creating or updating. Starts from 0, MUST GO IN SEQUENCE
   * @param offset The offset in the partiton where you want to insert or update
   * @param instance The instance data
   */
  void update(const unsigned int partition, const unsigned int offset, std::vector<Instance> &instances);

  /**
   * @param vboSize The size of the vertex array buffer object
   * @param eboSize The size of the element array buffer object
   */
  void resize(unsigned int vboSize, unsigned int eboSize);

  void bind() const;
  void unbind() const;
};