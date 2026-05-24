/**
 * @file Scene.hpp
 * @brief Scene class that manages all scene objects, including the camera.
 */

#pragma once
#include "Camera.hpp"

/**
 * @class Scene
 * @brief Acts as a container for the scene and provides a high-level interface for camera control.
 */
class Scene
{
private:
    Camera camera;     // The camera used to view the scene
    GLuint sphereSsbo; //

    void spawnSpheres()
    {
        int numSpheres = 4;
        Sphere spheres[numSpheres];
        spheres[0] = Sphere(glm::vec3(0.0, -55.0, -10.2), 54.2f, Material(glm::vec4(0.9, 0.9, 0.9, 1.0)));
        spheres[1] = Sphere(glm::vec3(-3.34, -0.2, -7.21), 0.63f, Material(glm::vec4(0.9, 0.1, 0.3, 1.0)));
        spheres[2] = Sphere(glm::vec3(-0.44, -0.0, -8.17), 1.0f, Material(glm::vec4(0.4, 0.8, 0.4, 1.0)));
        spheres[3] = Sphere(glm::vec3(-17.1, 0.28, -42.9), 22.75f, Material(glm::vec4(0), glm::vec3(1.0, 1.0, 1.0), 1.0f));

        // Create the buffer
        glGenBuffers(1, &sphereSsbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphereSsbo);

        // Allocate memory
        // Sphere aligns by 16. The first 16 includes an int and padding.
        int bufferSize = 16 + numSpheres * sizeof(Sphere);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);

        // Write data to the buffer
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int), &numSpheres);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 16, numSpheres * sizeof(Sphere), spheres);

        // Bind the buffer to a binding point
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sphereSsbo);
    }

public:
    /**
     * @brief Constructor for the Scene class.
     * @param shaderProgram The shader program used for rendering.
     */
    Scene(const GLuint shaderProgram) : camera(shaderProgram)
    {
        spawnSpheres();
    }

    /**
     * @brief Updates the camera's aspect ratio.
     * @param w New viewport width.
     * @param h New viewport height.
     */
    void setCameraAspectRatio(int w, int h)
    {
        camera.setAspect(w, h);
    }

    /**
     * @brief Moves the camera in the specified direction.
     * @param direction The direction to move the camera.
     */
    void moveCamera(glm::vec3 direction)
    {
        camera.move(direction);
    }

    /**
     * @brief Rotates the camera based on orientation offsets.
     * @param yawOffset Horizontal rotation offset.
     * @param pitchOffset Vertical rotation offset.
     */
    void rotateCamera(glm::vec2 yawPitch)
    {
        camera.rotate(yawPitch);
    }

    /**
     * @brief Sends scene-related data (like camera parameters) to the GPU.
     */
    void sendData()
    {
        camera.sendCameraData();
    }
};
