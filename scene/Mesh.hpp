#pragma once
#include "SceneTypes.hpp"
#include "../cyCodeBase/cyTriMesh.h"
#include "../cyCodeBase/cyBVH.h"
#include <iostream>
#include <string_view>
#include <vector>

/**
 * @class Mesh
 * @brief Handles loading of 3D models and stores mesh data (triangles and AABB).
 */
struct Mesh
{
    std::vector<Triangle> triangles;
    std::vector<BVHNode> nodes;
    AABB boundingBox;

    /**
     * @brief Loads a 3D model from an OBJ file.
     * @param filename Path to the OBJ file.
     * @return True if loading successful, false otherwise.
     */
    bool loadFromFileObj(std::string_view filename)
    {
        cy::TriMesh mesh;
        if (!mesh.LoadFromFileObj(filename.cbegin()))
        {
            std::cerr << "Failed to load model: " << filename << std::endl;
            return false;
        }

        cy::BVHTriMesh bvh(&mesh);
        // Triverse bvh to convert to our own type
        std::vector<uint32_t> sortedTriangleID = convertBVH(bvh);
        assert(sortedTriangleID.size() == mesh.NF());

        // Calculate AABB in local space using cyTriMesh
        mesh.ComputeBoundingBox();
        auto minB = mesh.GetBoundMin();
        auto maxB = mesh.GetBoundMax();
        boundingBox.min = glm::vec3(minB.x, minB.y, minB.z);
        boundingBox.max = glm::vec3(maxB.x, maxB.y, maxB.z);

        // Convert to Triangle structures
        for (int i = 0; i < mesh.NF(); i++)
        {
            uint32_t id = sortedTriangleID[i];
            auto face = mesh.F(id);
            Triangle tri;
            for (int j = 0; j < 3; j++)
            {
                // Position
                auto v = mesh.V(face.v[j]);
                tri.vertex[j] = glm::vec4(v.x, v.y, v.z, 1.0f);
                // Normal
                if (mesh.HasNormals())
                {
                    auto fn = mesh.FN(id);
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
                    auto ft = mesh.FT(id);
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

private:
    /**
     * @brief Converts a cy::BVHTriMesh to a local optimized BVHNode structure.
     * @param bvh The source BVH from cyCodeBase.
     * @return A vector of triangle indices sorted by the BVH layout.
     */
    std::vector<uint32_t> convertBVH(cy::BVHTriMesh &bvh)
    {
        // Clear old data
        std::vector<uint32_t> elementIDs;
        nodes.clear();

        // Count nodes and reserve
        uint32_t numNodes = countNodes(bvh, bvh.GetRootNodeID());
        nodes.resize(numNodes + 1); // +1 because cyBVH uses 1-based indexing for nodes

        // Build
        traverseBVH(bvh, bvh.GetRootNodeID(), elementIDs);

        return elementIDs;
    }

    /**
     * @brief Recursively counts the number of nodes in the BVH.
     */
    uint32_t countNodes(cy::BVHTriMesh &bvh, uint32_t nodeID)
    {
        if (bvh.IsLeafNode(nodeID))
            return 1;

        uint32_t childL, childR;
        bvh.GetChildNodes(nodeID, childL, childR);
        return 1 + countNodes(bvh, childL) + countNodes(bvh, childR);
    }

    /**
     * @brief Recursively traverses and builds the local BVH structure.
     */
    void traverseBVH(cy::BVHTriMesh &bvh, uint32_t nodeID, std::vector<uint32_t> &elementIDs)
    {
        // If leaf, set element data and bounds
        if (bvh.IsLeafNode(nodeID))
        {
            BVHNode &node = nodes[nodeID];
            node.childL = 0; // Leaf indicator (0 is unused in 1-based indexing)
            node.childR = 0; // Leaf indicator

            // Set node bounds
            const float *box = bvh.GetNodeBounds(nodeID);
            node.boundingBox.min = glm::vec3(box[0], box[1], box[2]);
            node.boundingBox.max = glm::vec3(box[3], box[4], box[5]);

            // Set element data
            node.elementOffset = static_cast<uint32_t>(elementIDs.size());
            node.elementNum = bvh.GetNodeElementCount(nodeID);

            // Copy element indices
            const uint32_t *elements = bvh.GetNodeElements(nodeID);
            for (int i = 0; i < (int)node.elementNum; i++)
                elementIDs.push_back(elements[i]);

            return;
        }

        // If internal, retrieve child IDs and set bounds
        uint32_t childL, childR;
        bvh.GetChildNodes(nodeID, childL, childR);

        BVHNode &node = nodes[nodeID];
        node.childL = childL;
        node.childR = childR;

        // Set node bounds
        const float *box = bvh.GetNodeBounds(nodeID);
        node.boundingBox.min = glm::vec3(box[0], box[1], box[2]);
        node.boundingBox.max = glm::vec3(box[3], box[4], box[5]);

        // Recurse to children
        traverseBVH(bvh, childL, elementIDs);
        traverseBVH(bvh, childR, elementIDs);
    }
};
