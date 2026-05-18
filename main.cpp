#include <GL/glew.h> // MUST be included before freeglut
#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std::literals;

// Global handles
GLuint computeProgram;
GLint workGroupSize[3];
GLuint textureOutput;
GLuint fboRescale; // Used for blitting the texture to the screen
int windowWidth = 640;
int windowHeight = 360;

void checkShaderCompile(GLuint shader)
{
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
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
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void initComputeShader(const std::string &file)
{
    // Load source from file
    std::cout << "Loading " << file << std::endl;
    std::string shaderSource = readShaderSource(file);
    const char *shaderPtr = shaderSource.c_str();

    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shader, 1, &shaderPtr, NULL);
    glCompileShader(shader);
    checkShaderCompile(shader);

    computeProgram = glCreateProgram();
    glAttachShader(computeProgram, shader);
    glLinkProgram(computeProgram);
    glDeleteShader(shader);

    glGetProgramiv(computeProgram, GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
}

void initTexture()
{
    // Create the texture that the compute shader will write to
    glGenTextures(1, &textureOutput);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);

    // Create a Framebuffer to "read" from the texture later during the blit
    glGenFramebuffers(1, &fboRescale);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboRescale);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureOutput, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void onKeyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:
        glutLeaveMainLoop();
        break;
    default:
        break;
    }
}

void idle()
{
    // Run Compute Shader
    glUseProgram(computeProgram);

    // Bind texture to image unit 0 (matches binding = 0 in shader)
    glBindImageTexture(0, textureOutput, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // Calculate work groups (ceiling division)
    GLuint groupX = (windowWidth + workGroupSize[0] - 1) / workGroupSize[0];
    GLuint groupY = (windowHeight + workGroupSize[1] - 1) / workGroupSize[1];
    glDispatchCompute(groupX, groupY, 1);

    // Make sure writing is finished before we try to read/display it
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glutPostRedisplay();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Display Result: Blit the texture from our FBO to the screen (default framebuffer 0)
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboRescale);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, windowWidth, windowHeight,
                      0, 0, windowWidth, windowHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);

    // Resize texture to match window
    glBindTexture(GL_TEXTURE_2D, textureOutput);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
}

int main(int argc, char **argv)
{
    // init GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitContextVersion(4, 6);
    glutInitContextFlags(GLUT_DEBUG);
    glutCreateWindow("Compute Shader Normalized Coords");

    // IMPORTANT: You must initialize GLEW after the window is created!
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
        return 1;
    }

    std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // init GL
    initComputeShader("shader/ray_tracing.glsl"s);
    initTexture();

    glutKeyboardFunc(onKeyboard);
    glutIdleFunc(idle);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glutMainLoop();
    return 0;
}