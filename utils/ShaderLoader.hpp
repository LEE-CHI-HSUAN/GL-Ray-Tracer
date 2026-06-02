/**
 * @file ShaderLoader.hpp
 * @brief Utilities for loading and compiling OpenGL shaders.
 */

#pragma once
#include <GL/glew.h>
#include <string>

/**
 * @brief Checks if a shader compiled successfully.
 * 
 * @param shader The OpenGL shader object to check.
 */
bool checkShaderCompile(GLuint shader);

/**
 * @brief Reads the source code of a shader from a file.
 * 
 * @param filePath Path to the shader source file.
 * @return std::string The content of the file.
 */
std::string readShaderSource(const std::string &filePath);

/**
 * @brief Initializes a compute shader program from a file.
 * 
 * @param file Path to the compute shader source file.
 * @return GLuint The linked shader program handle.
 */
GLuint initComputeShader(const std::string &file);
