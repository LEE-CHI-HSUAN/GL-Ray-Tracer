/**
 * @file SceneTypes.hpp
 * @brief Common data structures for the scene, shared between C++ and GLSL.
 */

#pragma once
#include "../glm/glm/glm.hpp"

/**
 * @struct CameraData
 * @brief Data structure for camera parameters, matching the std140 layout in GLSL.
 */
struct CameraData
{
    float nearClippingPlane = 0.1f;        // Distance to the near clipping plane
    float farClippingPlane = 100.0f;       // Distance to the far clipping plane
    float FoV = 60.0f;                     // Field of View in degrees
    float aspectRatio = 1.0f;              // Aspect ratio of the viewport
    glm::mat4 transform = glm::mat4(1.0f); // Camera-to-world transformation matrix
};

/**
 * @struct Sphere
 * @brief Data structure for sphere parameters.
 */
struct alignas(16) Sphere
{
    glm::vec3 center;
    float radius;
};
