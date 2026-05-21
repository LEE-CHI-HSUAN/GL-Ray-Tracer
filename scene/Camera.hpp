/**
 * @file Camera.hpp
 * @brief Camera class for managing camera state and transformation.
 */

#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "SceneTypes.hpp"
#include "../glm/glm/gtc/matrix_transform.hpp"
#include <algorithm>

/**
 * @enum CameraMovement
 * @brief Defines possible directions for camera movement.
 */
enum class CameraMovement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

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
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f); // Camera position in world space
    float yaw = -90.0f;                               // Yaw angle in degrees (initially looking down -Z)
    float pitch = 0.0f;                               // Pitch angle in degrees
    float movementSpeed = 0.1f;                       // Speed of camera movement
    float rotationSpeed = 1.0f;                       // Sensitivity of camera rotation

    /**
     * @brief Updates the camera transformation matrix based on position and orientation.
     */
    void updateTransformMatrix()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(front);

        // The transform matrix for the shader is the camera-to-world matrix (inverse of View matrix)
        camData.transform = glm::inverse(glm::lookAt(position, position + front, glm::vec3(0.0f, 1.0f, 0.0f)));
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
     * @brief Moves the camera in the specified direction.
     * @param direction The direction to move.
     */
    void move(CameraMovement direction)
    {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(front);

        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        if (direction == CameraMovement::FORWARD)
            position += front * movementSpeed;
        if (direction == CameraMovement::BACKWARD)
            position -= front * movementSpeed;
        if (direction == CameraMovement::LEFT)
            position -= right * movementSpeed;
        if (direction == CameraMovement::RIGHT)
            position += right * movementSpeed;
        if (direction == CameraMovement::UP)
            position += up * movementSpeed;
        if (direction == CameraMovement::DOWN)
            position -= up * movementSpeed;

        updateTransformMatrix();
    }

    /**
     * @brief Rotates the camera based on yaw and pitch offsets.
     * @param yawOffset The offset to apply to the yaw angle.
     * @param pitchOffset The offset to apply to the pitch angle.
     */
    void rotate(float yawOffset, float pitchOffset)
    {
        yaw += yawOffset * rotationSpeed;
        pitch += pitchOffset * rotationSpeed;

        // Clamp pitch to avoid gimbal lock
        pitch = std::clamp(pitch, -89.0f, 89.0f);

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
