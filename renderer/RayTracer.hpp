/**
 * @file RayTracer.hpp
 * @brief Class for managing ray tracing using OpenGL compute shaders.
 */

#pragma once
#include <GL/glew.h>
#include <string>

/**
 * @class RayTracer
 * @brief Handles compute shader execution and result display.
 */
class RayTracer
{
private:
    GLuint computeProgram;  ///< The linked compute shader program
    GLint workGroupSize[3]; ///< Work group size retrieved from the shader
    GLuint textureOutput;   ///< Texture where the compute shader writes results
    GLuint fboRescale;      ///< Framebuffer object for blitting the texture to the screen
    int windowWidth = 640;  ///< Current window width
    int windowHeight = 360; ///< Current window height

    /**
     * @brief Initializes the output texture and framebuffer.
     */
    void initTexture();

public:
    /**
     * @brief Constructor with default window size.
     * @param filePath Path to the compute shader source.
     */
    RayTracer(const std::string &filePath);

    /**
     * @brief Constructor with custom window size.
     * @param filePath Path to the compute shader source.
     * @param windowWidth Initial width of the window.
     * @param windowHeight Initial height of the window.
     */
    RayTracer(const std::string &filePath, const int windowWidth, const int windowHeight);

    /**
     * @brief Dispatches the compute shader to perform ray tracing.
     */
    void dispatchCompute();

    /**
     * @brief Updates the window size and resizes the output texture.
     * @param width New width.
     * @param height New height.
     * @return RayTracer* Pointer to this instance for chaining.
     */
    RayTracer *setWindowSize(int width, int height);

    /**
     * @brief Renders the result to the screen.
     */
    void displayScreen();

    /**
     * @brief Return the compute shader program.
     */
    GLuint getShaderProgram();
};
