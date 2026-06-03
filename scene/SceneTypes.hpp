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
struct alignas(16) Material
{
    glm::vec4 color = glm::vec4(1.0f);
    glm::vec3 emission_color = glm::vec3(0.0f);
    float emission_strength = 0.0f;
    float roughness = 0.2f;
    int32_t baseColorTextureID = -1;
    int32_t roughnessTextureID = -1;
    // int32_t _padding[1];
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
    // float _padding2 = 0.0f;
};

/**
 * @struct Model
 * @brief Data structure for model parameters, including AABB for optimization.
 */
struct alignas(16) Model
{
    glm::mat4 transform = glm::mat4(1.0f);
    int32_t nodeOffset;
    int32_t triangleOffset;
    // int32_t padding[2] = {0, 0};
    Material material;
};

/**
 * @struct BVHNode
 * @brief Represents a node in the Bounding Volume Hierarchy, aligned for GPU memory.
 */
struct alignas(16) BVHNode
{
    uint32_t childL;        // ID of the left child (0 if leaf)
    uint32_t childR;        // ID of the right child (0 if leaf)
    uint32_t elementOffset; // Offset into the elements buffer (valid only if leaf)
    uint32_t elementNum;    // Number of elements in this node (valid only if leaf)
    AABB boundingBox;       // Bounding box of this node
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
