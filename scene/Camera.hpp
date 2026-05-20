#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "SceneTypes.hpp"

class Camera
{
private:
    CameraData camData;
    unsigned int uboCamera;

public:
    Camera(const GLuint shaderProgram)
    {
        setAspect(glutGet(GLUT_WINDOW_X), glutGet(GLUT_WINDOW_Y));

        // create the buffer
        glGenBuffers(1, &uboCamera);
        glBindBuffer(GL_UNIFORM_BUFFER, uboCamera);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraData), NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // bind the buffer to a binding point (0)
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboCamera);

        // connect Shader to the Binding Point (0)
        unsigned int blockIndex = glGetUniformBlockIndex(shaderProgram, "CameraBlock");
        glUniformBlockBinding(shaderProgram, blockIndex, 0);
    }

    void setAspect(int w, int h)
    {
        camData.aspectRatio = (float)w / h;
    }

    void sendCameraData()
    {
        // 3. Update data (in your render loop)
        glBindBuffer(GL_UNIFORM_BUFFER, uboCamera);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraData), &camData);
    }
};
