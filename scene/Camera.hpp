/**
 * @file Camera.hpp
 * @brief Camera class for managing camera state and transformation.
 */

#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "SceneTypes.hpp"
#include "../glm/glm/gtc/matrix_transform.hpp"
#include "../glm/glm/gtc/quaternion.hpp"
#include <algorithm>

/**
 * @class Camera
 * @brief Handles camera parameters, movement, and orientation.
 */
class Camera
{
private:
    CameraData camData;     // Camera data sent to the GPU
    unsigned int uboCamera; // Uniform Buffer Object for camera data

    // Camera state for fly-through movement
    glm::vec3 position = glm::vec3();
    glm::quat orientation = glm::quat();

    // Cached direction vectors
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;

    // configuration
    float movementSpeed = 0.2f; // Speed of camera movement
    float rotationSpeed = 2.0f; // Sensitivity of camera rotation

    /**
     * @brief Updates the camera transformation matrix based on position and orientation.
     */
    void updateTransformMatrix()
    {
        orientation = glm::normalize(orientation);

        // Extract direction vectors from orientation
        front = orientation * glm::vec3(0.0f, 0.0f, -1.0f);
        right = orientation * glm::vec3(1.0f, 0.0f, 0.0f);
        up = orientation * glm::vec3(0.0f, 1.0f, 0.0f);

        // The transform matrix for the shader is the camera-to-world matrix
        glm::mat4 rotation = glm::mat4_cast(orientation);
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
        camData.transform = translation * rotation;
    }

public:
    /**
     * @brief Constructor for the Camera class.
     * @param shaderProgram The shader program to which the camera data will be bound.
     */
    Camera(const GLuint shaderProgram)
    {
        setAspect(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

        // Create the buffer
        glGenBuffers(1, &uboCamera);
        glBindBuffer(GL_UNIFORM_BUFFER, uboCamera);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraData), NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // Bind the buffer to a binding point (0)
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboCamera);

        // Connect Shader to the Binding Point (0)
        unsigned int blockIndex = glGetUniformBlockIndex(shaderProgram, "CameraBlock");
        glUniformBlockBinding(shaderProgram, blockIndex, 0);

        updateTransformMatrix();
    }

    /**
     * @brief Sets the aspect ratio of the camera based on window dimensions.
     * @param w Window width.
     * @param h Window height.
     */
    void setAspect(int w, int h)
    {
        if (h > 0)
            camData.aspectRatio = (float)w / h;
    }

    /**
     * @brief Moves the camera in the specified input.
     * @param input The movement in camera's local space.
     */
    void move(glm::vec3 input)
    {
        // transform input from local to global using cached vectors
        glm::vec3 worldMove = (right * input.x) + (up * input.y) + (front * input.z);
        position += worldMove * movementSpeed;

        updateTransformMatrix();
    }

    /**
     * @brief Rotates the camera based on yaw and pitch offsets.
     * @param yawOffset The offset to apply to the yaw angle.
     * @param pitchOffset The offset to apply to the pitch angle.
     */
    void rotate(glm::vec2 input)
    {
        float yawOffset = input.x, pitchOffset = input.y;

        // Horizontal rotation around the world UP axis
        glm::quat qYaw = glm::angleAxis(glm::radians(-yawOffset * rotationSpeed), glm::vec3(0.0f, 1.0f, 0.0f));
        // Vertical rotation around the local RIGHT axis
        glm::quat qPitch = glm::angleAxis(glm::radians(pitchOffset * rotationSpeed), glm::vec3(1.0f, 0.0f, 0.0f));

        // Apply rotations: Global Yaw * Current Orientation * Local Pitch
        orientation = qYaw * orientation * qPitch;

        updateTransformMatrix();
    }

    /**
     * @brief Sends updated camera data to the GPU.
     */
    void sendCameraData()
    {
        glBindBuffer(GL_UNIFORM_BUFFER, uboCamera);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraData), &camData);
    }
};
