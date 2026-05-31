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
 * @struct Material
 * @brief Data structure for Material parameters.
 */
struct Material
{
    glm::vec4 color = glm::vec4(1.0f);
    glm::vec3 emission_color = glm::vec4(0.0f);
    float emission_strength = 0.0f;

    Material() = default;
    Material(glm::vec4 color) : color(color) {}
    Material(glm::vec4 color, glm::vec3 emission_color, float emission_strength)
        : color(color),
          emission_color(emission_color),
          emission_strength(emission_strength) {}
};

/**
 * @struct Sphere
 * @brief Data structure for sphere parameters.
 */
struct alignas(16) Sphere
{
    glm::vec3 center = glm::vec3(0.0f);
    float radius = 1.0f;
    Material material;

    Sphere() = default;
    Sphere(glm::vec3 center, float radius, Material material)
        : center(center),
          radius(radius),
          material(material) {}
};

/**
 * @struct AABB
 * @brief Axis-Aligned Bounding Box, defined by min and max coordinates.
 */
struct alignas(16) AABB
{
    glm::vec3 min = glm::vec3(0.0f);
    float _padding1 = 0.0f;
    glm::vec3 max = glm::vec3(0.0f);
    float _padding2 = 0.0f;
};

/**
 * @struct Model
 * @brief Data structure for model parameters, including AABB for optimization.
 */
struct alignas(16) Model
{
    glm::mat4 transform = glm::mat4(1.0f);
    AABB boundingBox;
    int32_t start = 0;
    int32_t num_faces = 0;
    int32_t padding[2] = {0, 0};
};

/**
 * @struct Triangle
 * @brief Data structure for triangle parameters.
 */
struct alignas(16) Triangle
{
    glm::vec4 vertex[3];
    glm::vec4 normal[3];
    glm::vec2 uv[3];
};
