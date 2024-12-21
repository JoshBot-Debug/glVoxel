#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <unordered_map>

enum class ShaderType : unsigned int
{
  VERTEX_SHADER = GL_VERTEX_SHADER,
  FRAGMENT_SHADER = GL_FRAGMENT_SHADER
};

struct ShaderProgram
{
  std::string name;
  const char *vertex = nullptr;
  const char *fragment = nullptr;
};

struct Program
{
  unsigned int id;
  ShaderProgram shaderProgram;
};

class Shader
{
private:
  Program *program = nullptr;

  std::unordered_map<std::string, unsigned int> uniforms;
  std::unordered_map<std::string, Program> programs;
  std::unordered_map<std::string, unsigned int> vertexShaders;
  std::unordered_map<std::string, unsigned int> fragmentShaders;

public:
  ~Shader();

  /**
   * Recompiles the currently loaded shader. Typically used when the shader file
   * has been modified, and a fresh compilation is required.
   */
  void recompile();

  /**
   * Creates a shader
   * @param ShaderProgram The program you want to compile
   * @param force Force recompile even if shader was compiled previously
   */
  void create(ShaderProgram program, bool force = false);

  /**
   * Binds the specified shader program to the OpenGL pipeline.
   *
   * @param program The program ID of the shader to bind.
   */
  void bind(const std::string &name);

  /**
   * Unbinds the currently bound shader program, resetting to the default state.
   */
  void unbind();

  /**
   * Sets a uniform of type `int` in the currently active shader program.
   *
   * @param name The name of the uniform variable in the shader.
   * @param location The integer value to assign to the uniform variable.
   */
  void setUniform1i(const std::string &name, int location);

  /**
   * Sets a uniform of type `float` in the currently active shader program.
   *
   * @param name The name of the uniform variable in the shader.
   * @param value The float value to assign to the uniform variable.
   */
  void setUniform1f(const std::string &name, float value);

  /**
   * Sets a uniform of type `int` (two values) in the currently active shader program.
   *
   * @param name The name of the uniform variable in the shader.
   * @param value1 The first integer value.
   * @param value2 The second integer value.
   */
  void setUniform2i(const std::string &name, int value1, int value2);
  void setUniform2i(const std::string &name, glm::ivec2 value);

  /**
   * Sets a uniform of type `float` (two values) in the currently active shader program.
   *
   * @param name The name of the uniform variable in the shader.
   * @param value1 The first float value.
   * @param value2 The second float value.
   */
  void setUniform2f(const std::string &name, float value1, float value2);
  void setUniform2f(const std::string &name, glm::vec2 value);

  /**
   * Sets a uniform of type `int` (three values) in the currently active shader program.
   *
   * @param name The name of the uniform variable in the shader.
   * @param value1 The first integer value.
   * @param value2 The second integer value.
   * @param value3 The third integer value.
   */
  void setUniform3i(const std::string &name, int value1, int value2, int value3);
  void setUniform3i(const std::string &name, glm::ivec3 value);

  /**
   * Sets a uniform of type `float` (three values) in the currently active shader program.
   *
   * @param name The name of the uniform variable in the shader.
   * @param value1 The first float value.
   * @param value2 The second float value.
   * @param value3 The third float value.
   */
  void setUniform3f(const std::string &name, float value1, float value2, float value3);
  void setUniform3f(const std::string &name, glm::vec3 value);

  /**
   * Sets a uniform of type `int` (four values) in the currently active shader program.
   *
   * @param name The name of the uniform variable in the shader.
   * @param value1 The first integer value.
   * @param value2 The second integer value.
   * @param value3 The third integer value.
   * @param value4 The fourth integer value.
   */
  void setUniform4i(const std::string &name, int value1, int value2, int value3, int value4);
  void setUniform4i(const std::string &name, glm::ivec4 value);

  /**
   * Sets a uniform of type `float` (four values) in the currently active shader program.
   *
   * @param name The name of the uniform variable in the shader.
   * @param value1 The first float value.
   * @param value2 The second float value.
   * @param value3 The third float value.
   * @param value4 The fourth float value.
   */
  void setUniform4f(const std::string &name, float value1, float value2, float value3, float value4);
  void setUniform4f(const std::string &name, glm::vec4 value);

  /**
   * Sets a uniform of type `mat4` (4x4 matrix) in the currently active shader program.
   *
   * @param name The name of the uniform matrix variable in the shader.
   * @param matrix The 4x4 matrix (glm::mat4) to assign to the uniform variable.
   */
  void setUniformMatrix4fv(const std::string &name, const glm::mat4 &matrix);

  /**
   * Sets a uniform of type `mat3` (3x3 matrix) in the currently active shader program.
   *
   * @param name The name of the uniform matrix variable in the shader.
   * @param matrix The 3x3 matrix (glm::mat3) to assign to the uniform variable.
   */
  void setUniformMatrix3fv(const std::string &name, const glm::mat3 &matrix);

  /**
   * Sets a uniform of type `float` (1D vector) in the currently active shader program.
   *
   * @param name The name of the uniform vector variable in the shader.
   * @param vector The float to assign to the uniform variable.
   */
  void setUniform1fv(const std::string &name, float value);

  /**
   * Sets a uniform of type `vec2` (2D vector) in the currently active shader program.
   *
   * @param name The name of the uniform vector variable in the shader.
   * @param vector The 2D vector (glm::vec2) to assign to the uniform variable.
   */
  void setUniform2fv(const std::string &name, const glm::vec2 &vector);

  /**
   * Sets a uniform of type `vec3` (3D vector) in the currently active shader program.
   *
   * @param name The name of the uniform vector variable in the shader.
   * @param vector The 3D vector (glm::vec3) to assign to the uniform variable.
   */
  void setUniform3fv(const std::string &name, const glm::vec3 &vector);

  /**
   * Sets a uniform of type `vec4` (4D vector) in the currently active shader program.
   *
   * @param name The name of the uniform vector variable in the shader.
   * @param vector The 4D vector (glm::vec4) to assign to the uniform variable.
   */
  void setUniform4fv(const std::string &name, const glm::vec4 &vector);
};