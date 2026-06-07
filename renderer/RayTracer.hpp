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
    GLuint computeProgram;  // The linked compute shader program
    GLint workGroupSize[3]; // Work group size retrieved from the shader
    GLuint textures[2];     // Double buffers for accumulation
    int currentTexture = 0; // Index of the texture to write to this frame
    GLuint fboRescale;      // Framebuffer object for blitting the texture to the screen
    int windowWidth;        // Current window width
    int windowHeight;       // Current window height

    struct RenderParameters
    {
        float time;                 // Time in second
        int samplePerPixel = 10;    // SPP, number of rays per pixel in one dispatch
        int cumulative_samples = 0; // Number of SPP gathered for the current static scene
    };

    GLuint uboParameters;        // parameters data buffer
    RenderParameters parameters; // parameters data container
    int max_samples = 2000;      // Upper bound of SPP
    int savedFrameCount = 0;     // Counter for saved output images

    /**
     * @brief Initializes the compute shader program.
     * @param filePath Path to the compute shader source.
     */
    void initComputeShaderProgram(const std::string &filePath);

    /**
     * @brief Initializes the output texture and framebuffer.
     */
    void initTexture();

    /**
     * @brief Initializes the uniform buffer object for render parameters
     */
    void initParameterBuffer();

    /**
     * @brief Pass arguments to the shader program
     * @param time The elapsed time in seconds.
     */
    void sendRenderParameters(float time);

    /**
     * @brief Saves the current rendered texture to the output directory as a JPG.
     */
    void saveImage();

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
     * @param time The elapsed time in seconds.
     * @return `0`: no update, `1`: new screen rendered
     */
    int dispatchCompute(float time);

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
     * @brief Reset the counter of sampler. Call this when resizing screen or the screen is not static
     */
    void ResetRenderSpp();
    void ResetRenderSpp(int frameID);

    /**
     * @brief Return the compute shader program.
     */
    GLuint getShaderProgram();
};
