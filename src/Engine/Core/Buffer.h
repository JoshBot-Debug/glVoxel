#pragma once

#include <GL/glew.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

#include "Engine/Types.h"

enum class BufferTarget {
  ARRAY_BUFFER = GL_ARRAY_BUFFER,
  ELEMENT_ARRAY_BUFFER = GL_ELEMENT_ARRAY_BUFFER,
  DRAW_INDIRECT_BUFFER = GL_DRAW_INDIRECT_BUFFER,
};

struct BufferPartition {
  unsigned int size;  // size of the partition in bytes
  unsigned int used;  // bytes beign used
  unsigned int chunk; // size of one chunk in bytes

  BufferPartition(unsigned int size, unsigned int used, unsigned int chunk)
      : size(size), used(used), chunk(chunk) {}
};

class Buffer {
private:
  unsigned int m_Buffer = 0;

  BufferTarget m_Target;

  VertexDraw m_Draw;

  unsigned int m_ResizeFactor = 0;

  std::vector<unsigned int> m_Partitions;

public:
  /**
   * @param target Specify weather this is a vertex array buffer or an element
   * array buffer
   *
   * @param draw VertexDraw, Static for non changing, dynamic for frequently
   * changing, or stream
   */
  Buffer(BufferTarget target, VertexDraw draw = VertexDraw::STATIC);

  /**
   * @param target Specify weather this is a vertex array buffer or an element
   * array buffer
   *
   * @param resizeFactor This multiplier is applied to the incoming data size
   * when resizing a partition (e.g., during an upsert operation) to accommodate
   * more data. For example, if the data size is 12 bytes (3 floats) and the
   * buffer needs resizing, the buffer will be resized to accommodate 12 *
   * multiplier bytes.
   *
   * @param draw VertexDraw, Static for non changing, dynamic for frequently
   * changing, or stream
   */
  Buffer(BufferTarget target, unsigned int resizeFactor,
         VertexDraw draw = VertexDraw::STATIC);

  /**
   * Destructor for the Buffer.
   *
   * This function deletes the OpenGL buffer object (buffer) when the Buffer is
   * destroyed. It ensures that the buffer is properly cleaned up to free OpenGL
   * resources.
   *
   * If the buffer is valid (not zero), it calls `glDeleteBuffers` to delete the
   * buffer from the GPU memory.
   */
  ~Buffer();

  /**
   * Disable copy constructor
   */
  Buffer(const Buffer &) = delete;

  /**
   * Disable assignment operator
   */
  Buffer &operator=(const Buffer &) = delete;

  /**
   * Create a move constructor
   */
  Buffer(Buffer &&) = default;

  /**
   * Generates a buffer if it doesn't already exist.
   *
   * This function checks if the vertex buffer object (buffer) is already
   * generated, and if not, it calls OpenGL to generate one.
   */
  void generate();

  /**
   * Sets the buffer data with a vector of generic data type.
   *
   * @param data The data to store in the buffer. This is a vector of any type
   * T.
   * @param draw Specifies how the buffer will be used (static, dynamic, etc.).
   * Default is STATIC.
   *
   * @tparam T The type of the elements in the vector (e.g., float, glm::vec3).
   */
  template <typename T>
  void set(const std::vector<T> &data,
           const std::vector<unsigned int> partitions = {}) {
    unsigned int size = data.size() * sizeof(T);

    if (partitions.size())
      m_Partitions = partitions;
    else
      m_Partitions.emplace_back(size);

    glBindBuffer((unsigned int)m_Target, m_Buffer);
    glBufferData((unsigned int)m_Target, size, data.data(), (unsigned int)m_Draw);
  }

  /**
   * Sets the buffer data using raw data (void pointer).
   *
   * @param chunk The size of one chunk in bytes.
   * @param count The number of chunks you are inserting
   * @param data A pointer to the raw data to be uploaded to the buffer.
   * @param partitions A vector of buffer sizes
   */
  void set(unsigned int chunk, unsigned int count, const void *data,
           const std::vector<unsigned int> partitions = {});

  /**
   * Updates part of the buffer with a vector of generic data type.
   *
   * @param offset The offset in the buffer to start updating (in terms of
   * number of elements).
   * @param data The new data to upload to the buffer. This is a vector of any
   * type T.
   *
   * @tparam T The type of the elements in the vector (e.g., float, glm::vec3).
   */
  template <typename T>
  void update(const std::vector<T> &data, unsigned int offset = 0,
              unsigned int partition = 0) {
    glBufferSubData((unsigned int)m_Target,
                    offset * sizeof(T) +
                        getBufferPartitionOffsetSize(partition),
                    data.size() * sizeof(T), data.data());
  }

  /**
   * Updates part of the buffer using raw data.
   *
   * @param chunk The size of one chunk in bytes.
   * @param size The size of the data to update in bytes.
   * @param offset The offset of chunks in the buffer.
   * @param data A pointer to the raw data to upload to the buffer.
   * @param partition The partition you want to update the data into.
   */
  void update(unsigned int chunk, unsigned int size, const void *data,
              unsigned int offset = 0, unsigned int partition = 0);

  /**
   * Insert or updates part of the buffer with a vector of generic data type.
   * If there is not enough place in the partition, the buffer will resize.
   *
   * @param data The new data to upload to the buffer. This is a vector of any
   * type T.
   * @param offset The offset in the buffer to start updating (in terms of
   * number of elements).
   * @param partition The partition you want to upsert the data into.
   *
   * @tparam T The type of the elements in the vector (e.g., float, glm::vec3).
   */
  template <typename T>
  void upsert(const std::vector<T> &data, unsigned int offset = 0,
              unsigned int partition = 0) {
    // Did you forget to call .addPartition(0) before trying to upsert to a
    // partition that does not exist? You need to add a partition first. And if
    // only partition 0 exists, you cannot try upserting to partition[2,3,4,...]

    assert(m_Partitions.size() > partition);

    int dataSize = data.size() * sizeof(T);
    int offsetSize = offset * sizeof(T);
    int expansionSize = (offsetSize + dataSize) - m_Partitions[partition];

    if (expansionSize > 0)
      resize(partition, expansionSize + (dataSize * m_ResizeFactor), offsetSize);

    glBufferSubData((unsigned int)m_Target,
                    offsetSize + getBufferPartitionOffsetSize(partition),
                    dataSize, data.data());
  }

  /**
   * Insert or updates part of the buffer with a vector of generic data type.
   * If there is not enough place in the partition, the buffer will resize.
   *
   * @param chunk The size of one chunk in bytes.
   * @param size The size of the data to update in bytes.
   * @param offset The offset in the buffer to start updating (in terms of
   * number of elements).
   * @param data The new data to upload to the buffer. This is a vector of any
   * type T.
   * @param partition The partition you want to upsert the data into.
   *
   * @tparam T The type of the elements in the vector (e.g., float, glm::vec3).
   */
  void upsert(unsigned int chunk, unsigned int size, const void *data,
              unsigned int offset = 0, unsigned int partition = 0) {
    // Did you forget to call .addPartition(0) before trying to upsert to a
    // partition that does not exist? You need to add a partition first. And if
    // only partition 0 exists, you cannot try upserting to partition[2,3,4,...]
    assert(m_Partitions.size() > partition);

    int offsetSize = offset * chunk;
    int expansionSize = (offsetSize + size) - m_Partitions[partition];

    if (expansionSize > 0)
      resize(partition, expansionSize + (size * m_ResizeFactor), offsetSize);

    glBufferSubData((unsigned int)m_Target,
                    offsetSize + getBufferPartitionOffsetSize(partition), size,
                    data);
  }

  /**
   * Gets the OpenGL ID of the buffer.
   *
   * @return The OpenGL ID of the buffer.
   */
  unsigned int get() const;

  /**
   * Binds the buffer.
   */
  void bind() const;

  /**
   * Unbinds the buffer.
   */
  void unbind() const;

  /**
   * @returns the size of the buffer object in bytes.
   */
  int getBufferSize() const;

  /**
   * @returns the data in the buffer.
   */
  template <typename T> std::vector<T> getBufferData() const {
    const int size = getBufferSize();
    const int items = size / sizeof(T);

    std::vector<T> data(items);

    glGetBufferSubData((unsigned int)m_Target, 0, size, data.data());

    return data;
  };

  /**
   * @param partition The partition data that you want to retrieve
   * @returns the specific partition data in the buffer.
   */
  template <typename T> std::vector<T> getBufferData(unsigned int partition) {
    const unsigned int offset = getBufferPartitionOffsetSize(partition);
    const int size = m_Partitions[partition];
    const int items = size / sizeof(T);

    std::vector<T> data(items);

    glGetBufferSubData((unsigned int)m_Target, offset, size, data.data());

    return data;
  };

  /**
   * Expands a partition by resizing the buffer
   *
   * @param partition The partition that needs to be expanded
   * @param size The size (in bytes) by which you want to expand this partition
   * @param offset The offset (in bytes) by from where in the partition you want
   * to resize
   */
  void resize(unsigned int partition, unsigned int size,
              unsigned int offset = 0);

  /**
   * Creates a new partition. This function will resize the buffer
   *
   * @param size The size in bytes.
   */
  const unsigned int addPartition(unsigned int size);

  /**
   * Checks if the partiton sepcified is the next partition
   */
  const bool isNextPartition(const unsigned int partition);

  /**
   * Checks if the partition exists
   */
  const bool partitionExists(const unsigned int partition);

  size_t getBufferPartitionOffsetSize(unsigned int partitionIndex) {
    // Did you forget to call .addPartition(0) before trying to upsert to a
    // partition that does not exist? You need to add a partition first. And if
    // only partition 0 exists, you cannot try upserting or updating to
    // partition[2,3,4,...]
    assert(partitionExists(partitionIndex));

    size_t size = 0;

    for (unsigned int i = 0; i < partitionIndex; i++)
      size += m_Partitions[i];

    return size;
  }

  size_t getBufferPartitionSize(unsigned int partitionIndex) {
    // Did you forget to call .addPartition(0) before trying to upsert to a
    // partition that does not exist? You need to add a partition first. And if
    // only partition 0 exists, you cannot try upserting or updating to
    // partition[2,3,4,...]
    assert(partitionExists(partitionIndex));

    return m_Partitions[partitionIndex];
  }
};