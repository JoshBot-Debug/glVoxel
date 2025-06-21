#include "VertexArray.h"

VertexArray::~VertexArray() {
  if (m_Vao)
    glDeleteVertexArrays(1, &m_Vao);
}

void VertexArray::generate() {
  if (!m_Vao)
    glGenVertexArrays(1, &m_Vao);
}

void VertexArray::set(unsigned int index, unsigned int size, VertexType type,
                      bool normalized, size_t stride,
                      const void *pointer) const {
  if (type == VertexType::INT || type == VertexType::UNSIGNED_INT)
    glVertexAttribIPointer(index, size, (unsigned int)type, stride, pointer);
  else
    glVertexAttribPointer(index, size, (unsigned int)type,
                          normalized ? GL_TRUE : GL_FALSE, stride, pointer);
  glEnableVertexAttribArray(index);
}

void VertexArray::set(unsigned int index, unsigned int size, VertexType type,
                      bool normalized, size_t stride, const void *pointer,
                      unsigned int divisor) const {
  if (type == VertexType::INT || type == VertexType::UNSIGNED_INT)
    glVertexAttribIPointer(index, size, (unsigned int)type, stride, pointer);
  else
    glVertexAttribPointer(index, size, (unsigned int)type,
                          normalized ? GL_TRUE : GL_FALSE, stride, pointer);
  glEnableVertexAttribArray(index);
  glVertexAttribDivisor(index, divisor);
}

void VertexArray::bind() const { glBindVertexArray(m_Vao); }

void VertexArray::unbind() const { glBindVertexArray(0); }

unsigned int VertexArray::get() const { return m_Vao; }
