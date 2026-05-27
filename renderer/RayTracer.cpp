#include "RayTracer.hpp"
#include "../utils/ShaderLoader.hpp"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>

#pragma region Private Methods

void RayTracer::initTexture()
{
    // Create the textures that the compute shader will write to and read from
    glGenTextures(2, textures);

    for (int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);

        // Set texture parameters for clamping and linear filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        // Allocate texture memory with 32-bit float RGBA format
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    }

    // Create a Framebuffer to "read" from the texture later during the blit operation
    glGenFramebuffers(1, &fboRescale);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboRescale);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0], 0);

    // Unbind to avoid accidental modifications
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void RayTracer::initParameterBuffer()
{
    // Create the buffer
    glGenBuffers(1, &uboParameters);
    glBindBuffer(GL_UNIFORM_BUFFER, uboParameters);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(RenderParameters), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Bind the buffer to the binding point
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboParameters);
}

void RayTracer::sendRenderParameters(float time)
{
    parameters.time = time;
    glBindBuffer(GL_UNIFORM_BUFFER, uboParameters);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(RenderParameters), &parameters);
}

#pragma endregion Private Methods

#pragma region Constructors

RayTracer::RayTracer(const std::string &filePath)
{
    windowWidth = glutGet(GLUT_WINDOW_WIDTH);
    windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

    // Initialize the compute shader and retrieve its work group size
    computeProgram = initComputeShader(filePath);
    glGetProgramiv(computeProgram, GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
    initTexture();
    initParameterBuffer();
}

RayTracer::RayTracer(
    const std::string &filePath,
    const int windowWidth,
    const int windowHeight)
    : windowWidth(windowWidth), windowHeight(windowHeight)
{
    // Initialize with custom dimensions
    computeProgram = initComputeShader(filePath);
    glGetProgramiv(computeProgram, GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
    initTexture();
    initParameterBuffer();
}

#pragma endregion Constructors

#pragma region Public Methods

int RayTracer::dispatchCompute(float time)
{
    if (parameters.cumulative_samples >= max_samples)
        return 0;

    // Use the compute shader program
    glUseProgram(computeProgram);
    sendRenderParameters(time);
    parameters.cumulative_samples += parameters.samplePerPixel;

    // Bind the output texture to image unit 0 (must match 'binding = 0' in GLSL)
    glBindImageTexture(0, textures[currentTexture], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    // Bind the previous texture to image unit 1 (must match 'binding = 1' in GLSL)
    glBindImageTexture(1, textures[1 - currentTexture], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

    // Calculate number of work groups required to cover the entire window
    // Ceiling division is used to ensure all pixels are covered
    GLuint groupX = (windowWidth + workGroupSize[0] - 1) / workGroupSize[0];
    GLuint groupY = (windowHeight + workGroupSize[1] - 1) / workGroupSize[1];

    // Dispatch the compute shader
    glDispatchCompute(groupX, groupY, 1);

    // Insert a memory barrier to ensure all image writes are complete before sampling/displaying
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Swap textures for the next frame
    currentTexture = 1 - currentTexture;

    return 1;
}

RayTracer *RayTracer::setWindowSize(int width, int height)
{
    windowWidth = width;
    windowHeight = height;

    // Resize both textures to match the new window dimensions
    for (int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    }

    return this;
}

void RayTracer::displayScreen()
{
    // Clear the default framebuffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Display Result: Blit the texture from our internal FBO to the screen (default framebuffer 0)
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboRescale);
    // Attach the most recently rendered texture
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[1 - currentTexture], 0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    // Perform the blit with nearest-neighbor filtering
    glBlitFramebuffer(0, 0, windowWidth, windowHeight,
                      0, 0, windowWidth, windowHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Unbind to avoid accidental modifications
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void RayTracer::ResetRenderSpp()
{
    parameters.cumulative_samples = 0;
}

GLuint RayTracer::getShaderProgram()
{
    return computeProgram;
}

#pragma endregion Public Methods