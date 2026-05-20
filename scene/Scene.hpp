#pragma once
#include "Camera.hpp"

class Scene
{
private:
    Camera camera;

public:
    Scene(const GLuint shaderProgram) : camera(shaderProgram) {}

    void setCameraAspectRatio(int w, int h)
    {
        camera.setAspect(w, h);
    }

    void sendData()
    {
        camera.sendCameraData();
    }
};
