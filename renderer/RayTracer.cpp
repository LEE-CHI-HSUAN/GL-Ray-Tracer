#include "RayTracer.hpp"
#include "../utils/ShaderLoader.hpp"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <algorithm>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb/stb_image_write.h"

#pragma region Private Methods

void RayTracer::initComputeShaderProgram(const std::string &filePath)
{
    computeProgram = initComputeShader(filePath);
    if (computeProgram == 0) {
        std::cerr << "Failed to initialize compute shader." << std::endl;
        exit(1);
    }
    glGetProgramiv(computeProgram, GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
}

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

    initComputeShaderProgram(filePath);
    initTexture();
    initParameterBuffer();
}

RayTracer::RayTracer(
    const std::string &filePath,
    const int windowWidth,
    const int windowHeight)
    : windowWidth(windowWidth), windowHeight(windowHeight)
{
    initComputeShaderProgram(filePath);
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

    if (parameters.cumulative_samples >= max_samples)
    {
        saveImage();
    }

    return 1;
}

void RayTracer::saveImage()
{
    // Create the output directory if it doesn't exist
    std::filesystem::create_directories("output");

    // Retrieve texture dimensions
    std::vector<float> floatPixels(windowWidth * windowHeight * 4);

    // Bind texture and retrieve pixel data
    glBindTexture(GL_TEXTURE_2D, textures[1 - currentTexture]);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, floatPixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    // Convert from RGBA32F (floats) to RGB8 (unsigned char)
    std::vector<unsigned char> bytePixels(windowWidth * windowHeight * 3);
    for (int i = 0; i < windowWidth * windowHeight; ++i)
    {
        float r = floatPixels[i * 4 + 0];
        float g = floatPixels[i * 4 + 1];
        float b = floatPixels[i * 4 + 2];

        bytePixels[i * 3 + 0] = static_cast<unsigned char>(std::min(std::max(r, 0.0f), 1.0f) * 255.0f);
        bytePixels[i * 3 + 1] = static_cast<unsigned char>(std::min(std::max(g, 0.0f), 1.0f) * 255.0f);
        bytePixels[i * 3 + 2] = static_cast<unsigned char>(std::min(std::max(b, 0.0f), 1.0f) * 255.0f);
    }

    // Format the filename with 5 digits padded with 0 (e.g., 00000.jpg)
    std::stringstream ss;
    ss << "output/" << std::setfill('0') << std::setw(5) << savedFrameCount << ".jpg";
    std::string filepath = ss.str();

    // Flip vertically since OpenGL coordinate system is bottom-left
    stbi_flip_vertically_on_write(1);

    // Save image as JPG
    if (stbi_write_jpg(filepath.c_str(), windowWidth, windowHeight, 3, bytePixels.data(), 95))
    {
        std::cout << "Successfully saved rendered scene to: " << filepath << " at " << parameters.cumulative_samples << " samples." << std::endl;
        savedFrameCount++;
    }
    else
    {
        std::cerr << "Failed to save rendered scene to: " << filepath << std::endl;
    }
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