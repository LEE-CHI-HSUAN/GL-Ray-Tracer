/**
 * @file Scene.hpp
 * @brief Scene class that manages all scene objects, including the camera.
 */

#pragma once
#include "Camera.hpp"
#include "SceneTypes.hpp"
#include "../cyCodeBase/cyTriMesh.h"
#include "../glm/glm/gtc/matrix_transform.hpp"
#include <vector>
#include <iostream>

/**
 * @class Scene
 * @brief Acts as a container for the scene and provides a high-level interface for camera control.
 */
class Scene
{
private:
    Camera camera;     // The camera used to view the scene
    GLuint sphereSsbo; // Sphere data buffer
    GLuint modelSsbo;  // Model data buffer
    GLuint triangleSsbo; // Triangle data buffer

    std::vector<Triangle> triangles;
    std::vector<Model> models;

    /**
     * @brief Initializes and uploads hardcoded sphere data to the GPU.
     */
    void spawnSpheres()
    {
        int numSpheres = 4;
        Sphere spheres[numSpheres];
        spheres[0] = Sphere(glm::vec3(4.0, -55.0, -6.2), 54.2f, Material(glm::vec4(0.9, 0.9, 0.9, 1.0)));
        spheres[1] = Sphere(glm::vec3(0.66, -0.2, -3.21), 0.63f, Material(glm::vec4(0.8, 0.21, 0.17, 1.0)));
        spheres[2] = Sphere(glm::vec3(3.56, -0.0, -4.17), 1.0f, Material(glm::vec4(0.4, 0.76, 0.21, 1.0)));
        spheres[3] = Sphere(glm::vec3(-13.1, 0.28, -38.9), 22.75f, Material(glm::vec4(0), glm::vec3(1.0, 1.0, 1.0), 5.0f));

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

        // Bind the buffer to the binding point
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, sphereSsbo);
    }

    /**
     * @brief Uploads model and triangle data to the GPU buffers.
     */
    void uploadBuffers()
    {
        // Upload models
        if (modelSsbo == 0) glGenBuffers(1, &modelSsbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelSsbo);
        int bufferSizeModels = 16 + models.size() * sizeof(Model);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSizeModels, NULL, GL_STATIC_DRAW);
        int numModels = models.size();
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int), &numModels);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 16, numModels * sizeof(Model), models.data());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, modelSsbo);

        // Upload triangles
        if (triangleSsbo == 0) glGenBuffers(1, &triangleSsbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleSsbo);
        int bufferSizeTriangles = 16 + triangles.size() * sizeof(Triangle);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSizeTriangles, NULL, GL_STATIC_DRAW);
        int numTriangles = triangles.size();
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int), &numTriangles);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 16, numTriangles * sizeof(Triangle), triangles.data());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, triangleSsbo);
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
     * @brief Loads a 3D model from an OBJ file and adds it to the scene.
     * @param filename Path to the OBJ file.
     * @param position Translation vector for the model.
     * @param rotation Rotation angles for the model.
     * @param scale Scaling factors for the model.
     */
    void createModel(const std::string& filename, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
    {
        cy::TriMesh mesh;
        if (!mesh.LoadFromFileObj(filename.c_str())) {
            std::cerr << "Failed to load model: " << filename << std::endl;
            return;
        }

        Model model;
        model.start = triangles.size();
        model.num_faces = mesh.NF();

        // Calculate AABB in local space using cyTriMesh
        mesh.ComputeBoundingBox();
        auto minB = mesh.GetBoundMin();
        auto maxB = mesh.GetBoundMax();
        model.boundingBox.min = glm::vec3(minB.x, minB.y, minB.z);
        model.boundingBox.max = glm::vec3(maxB.x, maxB.y, maxB.z);

        // Calculate transformation matrix
        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::translate(trans, position);
        trans = glm::rotate(trans, glm::radians(rotation.x), glm::vec3(1, 0, 0));
        trans = glm::rotate(trans, glm::radians(rotation.y), glm::vec3(0, 1, 0));
        trans = glm::rotate(trans, glm::radians(rotation.z), glm::vec3(0, 0, 1));
        trans = glm::scale(trans, scale);
        model.transform = trans;

        models.push_back(model);

        // Append model's triangles to the triangles array
        for (int i = 0; i < mesh.NF(); i++) {
            auto face = mesh.F(i);
            Triangle tri;
            for (int j = 0; j < 3; j++) {
                // Position
                auto v = mesh.V(face.v[j]);
                tri.vertex[j] = glm::vec4(v.x, v.y, v.z, 1.0f);
                // Normal
                if (mesh.HasNormals()) {
                    auto fn = mesh.FN(i);
                    auto vn = mesh.VN(fn.v[j]);
                    tri.normal[j] = glm::vec4(vn.x, vn.y, vn.z, 0.0f);
                } else {
                    tri.normal[j] = glm::vec4(0.0f);
                }
                // UV, aka texture coordinate
                if (mesh.HasTextureVertices()) {
                    auto ft = mesh.FT(i);
                    auto vt = mesh.VT(ft.v[j]);
                    tri.uv[j] = glm::vec2(vt.x, vt.y);
                } else {
                    tri.uv[j] = glm::vec2(0.0f);
                }
            }
            triangles.push_back(tri);
        }
        uploadBuffers();
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
