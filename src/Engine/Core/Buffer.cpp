#include "Buffer.h"
#include "Debug.h"

Buffer::Buffer(BufferTarget target, VertexDraw draw) : target(target), draw(draw) {}

Buffer::Buffer(BufferTarget target, unsigned int resizeFactor, VertexDraw draw) : target(target), draw(draw), resizeFactor(resizeFactor) {};

Buffer::~Buffer()
{
  if (buffer)
    glDeleteBuffers(1, &buffer);
}

void Buffer::generate()
{
  if (!buffer)
    glGenBuffers(1, &buffer);
}

void Buffer::set(unsigned int chunk, unsigned int count, const void *data, const std::vector<unsigned int> partitions)
{
  unsigned int size = count * chunk;

  if (partitions.size())
    this->partitions = partitions;
  else
    this->partitions.emplace_back(size);

  glBindBuffer((unsigned int)target, buffer);
  glBufferData((unsigned int)target, size, data, (unsigned int)draw);
}

void Buffer::update(unsigned int chunk, unsigned int offset, unsigned int size, const void *data, unsigned int partition)
{
  glBufferSubData((unsigned int)target, (offset * chunk) + getBufferPartitionOffsetSize(partition), size, data);
}

unsigned int Buffer::get() const { return buffer; }

void Buffer::bind() const { glBindBuffer((unsigned int)target, buffer); }

void Buffer::unbind() const { glBindBuffer((unsigned int)target, 0); }

int Buffer::getBufferSize() const
{
  glBindBuffer((unsigned int)target, buffer);

  int size = 0;
  glGetBufferParameteriv((unsigned int)target, GL_BUFFER_SIZE, &size);

  return size;
}

void Buffer::resize(unsigned int size)
{
  int currentSize;
  glBindBuffer(GL_COPY_READ_BUFFER, buffer);
  glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &currentSize);

  unsigned int next;
  glGenBuffers(1, &next);
  glBindBuffer((unsigned int)target, next);
  glBufferData((unsigned int)target, size, nullptr, (unsigned int)draw);

  glBindBuffer(GL_COPY_WRITE_BUFFER, next);
  glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, currentSize);
  glDeleteBuffers(1, &buffer);

  buffer = next;
}

void Buffer::resize(unsigned int partition, unsigned int size, unsigned int offset)
{
  int currentSize;
  glBindBuffer(GL_COPY_READ_BUFFER, buffer);
  glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &currentSize);

  unsigned int next;
  glGenBuffers(1, &next);
  glBindBuffer((unsigned int)target, next);
  glBufferData((unsigned int)target, currentSize + size, nullptr, (unsigned int)draw);

  glBindBuffer(GL_COPY_WRITE_BUFFER, next);

  int partitionOffset = getBufferPartitionOffsetSize(partition);

  const size_t remaining = currentSize - (partitionOffset + offset);

  if (partitionOffset + offset > 0)
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, partitionOffset + offset);

  if (remaining > 0)
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, partitionOffset + offset, partitionOffset + offset + size, remaining);

  glBindBuffer(GL_COPY_READ_BUFFER, 0);
  glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
  glDeleteBuffers(1, &buffer);

  buffer = next;

  partitions[partition] += size;
}

const unsigned int Buffer::addPartition(unsigned int size)
{
  this->partitions.emplace_back(size);
  const unsigned int id = this->partitions.size() - 1;

  if (size > 0)
    resize(id, size, 0);

  return id;
}

const bool Buffer::isNextPartition(const unsigned int partition)
{
  return this->partitions.size() == partition;
}

const bool Buffer::partitionExists(const unsigned int partition)
{
  return partitions.size() > partition;
}
