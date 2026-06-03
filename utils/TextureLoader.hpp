#pragma once
#include <GL/glew.h>
#include <string>

/**
 * @brief Loads an image from a file and returns an OpenGL texture ID.
 * @param filename Path to the image file.
 * @return OpenGL texture ID, or 0 if loading failed.
 */
GLuint loadTexture(const std::string &filename);
