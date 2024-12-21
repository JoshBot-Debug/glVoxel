#include "MultiModelInstanceBuffer.h"

MultiModelInstanceBuffer::MultiModelInstanceBuffer() : vbo(BufferTarget::ARRAY_BUFFER), ebo(BufferTarget::ELEMENT_ARRAY_BUFFER, VertexDraw::DYNAMIC), ibo(BufferTarget::ARRAY_BUFFER, VertexDraw::DYNAMIC)
{
  vao.generate();
  vbo.generate();
  ebo.generate();
  ibo.generate();
}

const unsigned int MultiModelInstanceBuffer::addBufferData(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, unsigned int &firstIndex, unsigned int &baseVertex)
{
  const unsigned int epID = ebo.addPartition(0);
  const unsigned int vpID = vbo.addPartition(0);

  assert(epID == vpID);

  ebo.upsert(0, indices, epID);
  firstIndex = ebo.getBufferPartitionOffsetSize(epID) / sizeof(unsigned int);

  vbo.upsert(0, vertices, vpID);
  baseVertex = vbo.getBufferPartitionOffsetSize(vpID) / sizeof(Vertex);

  vao.set(0, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, position));
  vao.set(1, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, normal));
  vao.set(2, 2, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));
  vao.set(3, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, tangent));
  vao.set(4, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, bitangent));
  vao.set(5, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, color));

  return epID;
}

unsigned int MultiModelInstanceBuffer::add(const unsigned int partition, const Instance &instance, unsigned int &baseInstance)
{
  if (ibo.isNextPartition(partition))
    ibo.addPartition(0);

  const unsigned int offset = ibo.getBufferPartitionSize(partition) / sizeof(Instance);

  ibo.upsert(sizeof(Instance), offset, sizeof(instance), (const void *)&instance, partition);
  baseInstance = ibo.getBufferPartitionOffsetSize(partition) / sizeof(Instance);

  vao.bind();
  vao.set(6, 4, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)offsetof(InstanceBuffer, model), 1);
  vao.set(7, 4, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)(offsetof(InstanceBuffer, model) + (sizeof(float) * 4)), 1);
  vao.set(8, 4, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)(offsetof(InstanceBuffer, model) + (sizeof(float) * 8)), 1);
  vao.set(9, 4, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)(offsetof(InstanceBuffer, model) + (sizeof(float) * 12)), 1);
  vao.set(10, 3, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)offsetof(InstanceBuffer, normalMatrix), 1);
  vao.set(11, 3, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)(offsetof(InstanceBuffer, normalMatrix) + (sizeof(float) * 3)), 1);
  vao.set(12, 3, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)(offsetof(InstanceBuffer, normalMatrix) + (sizeof(float) * 6)), 1);
  vao.set(13, 3, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)offsetof(InstanceBuffer, color), 1);

  return offset;
}

std::vector<unsigned int> MultiModelInstanceBuffer::add(const unsigned int partition, const std::vector<Instance> &instances, unsigned int &baseInstance)
{
  if (ibo.isNextPartition(partition))
    ibo.addPartition(0);

  const unsigned int offset = ibo.getBufferPartitionSize(partition) / sizeof(Instance);

  ibo.upsert(offset, instances, partition);
  baseInstance = ibo.getBufferPartitionOffsetSize(partition) / sizeof(Instance);

  vao.bind();
  vao.set(6, 4, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)offsetof(InstanceBuffer, model), 1);
  vao.set(7, 4, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)(offsetof(InstanceBuffer, model) + (sizeof(float) * 4)), 1);
  vao.set(8, 4, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)(offsetof(InstanceBuffer, model) + (sizeof(float) * 8)), 1);
  vao.set(9, 4, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)(offsetof(InstanceBuffer, model) + (sizeof(float) * 12)), 1);
  vao.set(10, 3, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)offsetof(InstanceBuffer, normalMatrix), 1);
  vao.set(11, 3, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)(offsetof(InstanceBuffer, normalMatrix) + (sizeof(float) * 3)), 1);
  vao.set(12, 3, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)(offsetof(InstanceBuffer, normalMatrix) + (sizeof(float) * 6)), 1);
  vao.set(13, 3, VertexType::FLOAT, false, sizeof(InstanceBuffer), (void *)offsetof(InstanceBuffer, color), 1);

  std::vector<unsigned int> offsets(instances.size());

  for (size_t i = 0; i < instances.size(); i++)
    offsets.push_back(offset + i);

  return offsets;
}

void MultiModelInstanceBuffer::update(const unsigned int partition, const unsigned int offset, Instance &instance)
{
  assert(ibo.partitionExists(partition));
  assert(offset < ibo.getBufferPartitionSize(partition));

  const InstanceBuffer buffer = instance.update();
  ibo.upsert(sizeof(Instance), offset, sizeof(buffer), (const void *)&buffer, partition);
}

void MultiModelInstanceBuffer::update(const unsigned int partition, const unsigned int offset, std::vector<Instance> &instances)
{
  assert(ibo.partitionExists(partition));
  assert(offset < ibo.getBufferPartitionSize(partition));

  std::vector<InstanceBuffer> buffers;

  for (auto &instance : instances)
    buffers.push_back(instance.update());

  ibo.upsert(offset, buffers, partition);
}

void MultiModelInstanceBuffer::resize(unsigned int vboSize, unsigned int eboSize)
{
  vbo.resize(vboSize);
  ebo.resize(eboSize);
}

void MultiModelInstanceBuffer::bind() const
{
  vao.bind();
  ibo.bind();
}

void MultiModelInstanceBuffer::unbind() const
{
  vbo.unbind();
  ebo.unbind();
  ibo.unbind();
  vao.unbind();
}