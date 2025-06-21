#include "Buffer.h"
#include "Debug.h"

Buffer::Buffer(BufferTarget target, VertexDraw draw)
    : m_Target(target), m_Draw(draw) {}

Buffer::Buffer(BufferTarget target, unsigned int resizeFactor, VertexDraw draw)
    : m_Target(target), m_Draw(draw), m_ResizeFactor(resizeFactor){};

Buffer::~Buffer() {
  if (m_Buffer)
    glDeleteBuffers(1, &m_Buffer);
}

void Buffer::generate() {
  if (!m_Buffer)
    glGenBuffers(1, &m_Buffer);
}

void Buffer::set(unsigned int chunk, unsigned int count, const void *data,
                 const std::vector<unsigned int> partitions) {
  unsigned int size = count * chunk;

  if (partitions.size())
    m_Partitions = partitions;
  else
    m_Partitions.emplace_back(size);

  glBindBuffer((unsigned int)m_Target, m_Buffer);
  glBufferData((unsigned int)m_Target, size, data, (unsigned int)m_Draw);
}

void Buffer::update(unsigned int chunk, unsigned int size, const void *data,
                    unsigned int offset, unsigned int partition) {
  glBufferSubData((unsigned int)m_Target,
                  (offset * chunk) + getBufferPartitionOffsetSize(partition),
                  size, data);
}

unsigned int Buffer::get() const { return m_Buffer; }

void Buffer::bind() const { glBindBuffer((unsigned int)m_Target, m_Buffer); }

void Buffer::unbind() const { glBindBuffer((unsigned int)m_Target, 0); }

int Buffer::getBufferSize() const {
  glBindBuffer((unsigned int)m_Target, m_Buffer);

  int size = 0;
  glGetBufferParameteriv((unsigned int)m_Target, GL_BUFFER_SIZE, &size);

  return size;
}

void Buffer::resize(unsigned int partition, unsigned int size,
                    unsigned int offset) {
  if (m_Partitions.size() <= partition)
    addPartition(partition);

  int currentSize;
  glBindBuffer(GL_COPY_READ_BUFFER, m_Buffer);
  glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &currentSize);

  unsigned int next;
  glGenBuffers(1, &next);
  glBindBuffer((unsigned int)m_Target, next);
  glBufferData((unsigned int)m_Target, currentSize + size, nullptr,
               (unsigned int)m_Draw);

  glBindBuffer(GL_COPY_WRITE_BUFFER, next);

  int partitionOffset = getBufferPartitionOffsetSize(partition);

  const size_t remaining = currentSize - (partitionOffset + offset);

  if (partitionOffset + offset > 0)
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0,
                        partitionOffset + offset);

  if (remaining > 0)
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
                        partitionOffset + offset,
                        partitionOffset + offset + size, remaining);

  glBindBuffer(GL_COPY_READ_BUFFER, 0);
  glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
  glDeleteBuffers(1, &m_Buffer);

  m_Buffer = next;

  m_Partitions[partition] += size;
}

const unsigned int Buffer::addPartition(unsigned int size) {
  m_Partitions.emplace_back(size);
  const unsigned int id = m_Partitions.size() - 1;

  if (size > 0)
    resize(id, size, 0);

  return id;
}

const bool Buffer::isNextPartition(const unsigned int partition) {
  return m_Partitions.size() == partition;
}

const bool Buffer::partitionExists(const unsigned int partition) {
  return m_Partitions.size() > partition;
}
