#include "ShaderLoader.hpp"
#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
#include <sstream>

void checkShaderCompile(GLuint shader)
{
    GLint success;
    // Check if compilation was successful
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        // Retrieve and print the error log
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader Compilation Error:\n"
                  << infoLog << std::endl;
    }
}

std::string readShaderSource(const std::string &filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open shader file: " << filePath << std::endl;
        return "";
    }
    // Read the entire file content into a string stream
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint initComputeShader(const std::string &file)
{
    // Load source from file
    std::cout << "Loading " << file << std::endl;
    std::string shaderSource = readShaderSource(file);
    const char *shaderPtr = shaderSource.c_str();

    // Create and compile the compute shader
    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shader, 1, &shaderPtr, NULL);
    glCompileShader(shader);
    checkShaderCompile(shader);

    // Create the shader program and link the shader
    GLuint computeProgram = glCreateProgram();
    glAttachShader(computeProgram, shader);
    glLinkProgram(computeProgram);

    // Clean up the shader object as it's now linked into the program
    glDeleteShader(shader);

    return computeProgram;
}
