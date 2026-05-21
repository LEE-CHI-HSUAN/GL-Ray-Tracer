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
    Camera camera; ///< The camera used to view the scene

public:
    /**
     * @brief Constructor for the Scene class.
     * @param shaderProgram The shader program used for rendering.
     */
    Scene(const GLuint shaderProgram) : camera(shaderProgram) {}

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
    void moveCamera(CameraMovement direction)
    {
        camera.move(direction);
    }

    /**
     * @brief Rotates the camera based on orientation offsets.
     * @param yawOffset Horizontal rotation offset.
     * @param pitchOffset Vertical rotation offset.
     */
    void rotateCamera(float yawOffset, float pitchOffset)
    {
        camera.rotate(yawOffset, pitchOffset);
    }

    /**
     * @brief Sends scene-related data (like camera parameters) to the GPU.
     */
    void sendData()
    {
        camera.sendCameraData();
    }
};
