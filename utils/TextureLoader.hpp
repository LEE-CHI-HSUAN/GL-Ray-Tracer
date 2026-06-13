#pragma once
#include <GL/glew.h>
#include <string_view>

/**
 * @brief Loads an image from a file and returns an OpenGL texture ID.
 * @param filename Path to the image file.
 * @return OpenGL texture ID, or 0 if loading failed.
 */
GLuint loadTexture(std::string_view filename);
