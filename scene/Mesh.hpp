#pragma once
#include "../cyCodeBase/cyTriMesh.h"
#include "SceneTypes.hpp"
#include <vector>
#include <string>
#include <iostream>

/**
 * @class Mesh
 * @brief Handles loading of 3D models and stores mesh data (triangles and AABB).
 */
struct Mesh
{
    std::vector<Triangle> triangles;
    AABB boundingBox;

    /**
     * @brief Loads a 3D model from an OBJ file.
     * @param filename Path to the OBJ file.
     * @return True if loading successful, false otherwise.
     */
    bool loadFromFileObj(const std::string &filename)
    {
        cy::TriMesh mesh;
        if (!mesh.LoadFromFileObj(filename.c_str()))
        {
            std::cerr << "Failed to load model: " << filename << std::endl;
            return false;
        }

        // Calculate AABB in local space using cyTriMesh
        mesh.ComputeBoundingBox();
        auto minB = mesh.GetBoundMin();
        auto maxB = mesh.GetBoundMax();
        boundingBox.min = glm::vec3(minB.x, minB.y, minB.z);
        boundingBox.max = glm::vec3(maxB.x, maxB.y, maxB.z);

        // Convert to Triangle structures
        for (int i = 0; i < mesh.NF(); i++)
        {
            auto face = mesh.F(i);
            Triangle tri;
            for (int j = 0; j < 3; j++)
            {
                // Position
                auto v = mesh.V(face.v[j]);
                tri.vertex[j] = glm::vec4(v.x, v.y, v.z, 1.0f);
                // Normal
                if (mesh.HasNormals())
                {
                    auto fn = mesh.FN(i);
                    auto vn = mesh.VN(fn.v[j]);
                    tri.normal[j] = glm::vec4(vn.x, vn.y, vn.z, 0.0f);
                }
                else
                {
                    tri.normal[j] = glm::vec4(0.0f);
                }
                // UV, aka texture coordinate
                if (mesh.HasTextureVertices())
                {
                    auto ft = mesh.FT(i);
                    auto vt = mesh.VT(ft.v[j]);
                    tri.uv[j] = glm::vec2(vt.x, vt.y);
                }
                else
                {
                    tri.uv[j] = glm::vec2(0.0f);
                }
            }
            triangles.push_back(tri);
        }
        return true;
    }
};
