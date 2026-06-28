/**
 * @file Scene.hpp
 * @brief Scene class that manages all scene objects, including the camera.
 */

#pragma once
#include "Animation.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"
#include "SceneTypes.hpp"
#include "utils/TextureLoader.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @class Scene
 * @brief Acts as a container for the scene and provides a high-level interface for camera control.
 */
class Scene
{
private:
    /**
     * @struct MeshReference
     * @brief References mesh data to avoid redundant loading.
     */
    struct MeshReference
    {
        int nodeOffset;
        int triangleOffset;
    };

    Camera camera;       // The camera used to view the scene
    GLuint sphereSsbo;   // Sphere data buffer
    GLuint modelSsbo;    // Model data buffer
    GLuint nodeSsbo;     // BVH node data buffer
    GLuint triangleSsbo; // Triangle data buffer

    std::vector<Sphere> spheres;     // Object instances
    std::vector<Model> models;       // Object instances
    std::vector<BVHNode> nodes;      // Global BVH node array
    std::vector<Triangle> triangles; // Global triangle array
    std::unordered_map<std::string, MeshReference> meshReferenceMap;
    std::vector<Animation> sphereAnimations;
    std::vector<Animation> modelAnimations;

    std::vector<GLuint> textureIDs;
    std::unordered_map<std::string, int> textureMap;

    /**
     * @brief Initializes and uploads hardcoded sphere data to the GPU.
     */
    void spawnSpheres()
    {
        int numSpheres = 4;
        this->spheres.resize(numSpheres);
        this->spheres[0] = Sphere(glm::vec3(0.0, -55.0, -6.2), 54.2f, Material{.color = glm::vec4(0.9, 0.9, 0.9, 1.0), .roughness = 0.9});
        this->spheres[1] = Sphere(glm::vec3(-1.56, -0.2, -3.21), 0.63f, Material{.color = glm::vec4(0.8, 0.21, 0.17, 1.0), .roughness = 0.6});
        this->spheres[2] = Sphere(glm::vec3(1.66, -0.0, -4.17), 1.0f, Material{.color = glm::vec4(0.4, 0.76, 0.21, 1.0), .roughness = 0.4});
        this->spheres[3] = Sphere(glm::vec3(0.0, -30.0, -38.9), 22.75f, Material{.color = glm::vec4(0), .emission_color = glm::vec3(1.0, 1.0, 1.0), .emission_strength = 5.0f});

        sphereAnimations.resize(numSpheres);
        sphereAnimations[3].addKeyframe(Keyframe{.time = 0, .position = glm::vec3(0.0, -30.0, -38.9)});
        sphereAnimations[3].addKeyframe(Keyframe{.time = 8, .position = glm::vec3(0.0, 0.28, -35.9)});
        sphereAnimations[3].addKeyframe(Keyframe{.time = 15, .position = glm::vec3(0.0, 25.50, -25.0)});
        sphereAnimations[3].addKeyframe(Keyframe{.time = 20, .position = glm::vec3(0.0, 50.50, 0.0)});

        // Create the buffer
        glGenBuffers(1, &sphereSsbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphereSsbo);

        // Allocate memory
        // Sphere aligns by 16. The first 16 includes an int and padding.
        int bufferSize = 16 + numSpheres * sizeof(Sphere);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);

        // Write data to the buffer
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int), &numSpheres);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 16, numSpheres * sizeof(Sphere), spheres.data());

        // Bind the buffer to the binding point
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, sphereSsbo);
    }

    void uploadSphereBuffer()
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphereSsbo);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 16, spheres.size() * sizeof(Sphere), spheres.data());
    }

    void uploadModelBuffer()
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelSsbo);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 16, models.size() * sizeof(Model), models.data());
    }

    /**
     * @brief Uploads model, BVH node, and triangle data to the GPU buffers.
     */
    void uploadBuffers()
    {
        // Upload models
        if (modelSsbo == 0)
            glGenBuffers(1, &modelSsbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, modelSsbo);
        int bufferSizeModels = 16 + models.size() * sizeof(Model);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSizeModels, nullptr, GL_STATIC_DRAW);
        int numModels = models.size();
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int), &numModels);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 16, numModels * sizeof(Model), models.data());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, modelSsbo);

        // Upload BVH nodes
        if (nodeSsbo == 0)
            glGenBuffers(1, &nodeSsbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, nodeSsbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, nodes.size() * sizeof(BVHNode), nodes.data(), GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, nodeSsbo);

        // Upload triangles
        if (triangleSsbo == 0)
            glGenBuffers(1, &triangleSsbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleSsbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, triangles.size() * sizeof(Triangle), triangles.data(), GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, triangleSsbo);
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
     * @param meshFile Path to the OBJ file.
     * @param position Translation vector for the model.
     * @param rotation Rotation angles for the model.
     * @param scale Scaling factors for the model.
     * @param material Material properties for the model.
     * @param baseColorTexture Optional path to the base color texture.
     * @param roughnessTexture Optional path to the roughness texture.
     */
    void createModel(
        const std::string &meshFile,
        glm::vec3 position,
        glm::vec3 rotation,
        glm::vec3 scale,
        Material material,
        const std::string &baseColorTexture = "",
        const std::string &roughnessTexture = "")
    {
        if (!baseColorTexture.empty())
        {
            auto it = textureMap.find(baseColorTexture);
            if (it != textureMap.end())
            {
                material.baseColorTextureID = it->second;
            }
            else
            {
                GLuint textureID = loadTexture(baseColorTexture);
                if (textureID != 0)
                {
                    textureIDs.push_back(textureID);
                    material.baseColorTextureID = textureIDs.size() - 1;
                    textureMap[baseColorTexture] = material.baseColorTextureID;
                }
            }
        }

        if (!roughnessTexture.empty())
        {
            auto it = textureMap.find(roughnessTexture);
            if (it != textureMap.end())
            {
                material.roughnessTextureID = it->second;
            }
            else
            {
                GLuint textureID = loadTexture(roughnessTexture);
                if (textureID != 0)
                {
                    textureIDs.push_back(textureID);
                    material.roughnessTextureID = textureIDs.size() - 1;
                    textureMap[roughnessTexture] = material.roughnessTextureID;
                }
            }
        }

        // Check if the model is already loaded and referenced
        auto it = meshReferenceMap.find(meshFile);
        bool meshLoaded = (it != meshReferenceMap.end());

        Model model;
        model.material = material;

        if (meshLoaded)
        {
            // Reuse loaded data
            model.nodeOffset = it->second.nodeOffset;
            model.triangleOffset = it->second.triangleOffset;
        }
        else
        {
            // Load new model
            Mesh mesh;
            if (!mesh.loadFromFileObj(meshFile))
                return;

            model.nodeOffset = nodes.size();
            model.triangleOffset = triangles.size();

            // Update map
            meshReferenceMap[meshFile] = {model.nodeOffset, model.triangleOffset};

            // Append model's BVH nodes to the nodes array
            nodes.insert(nodes.end(),
                         std::make_move_iterator(mesh.nodes.begin()),
                         std::make_move_iterator(mesh.nodes.end()));

            // Append model's triangles to the triangles array
            triangles.insert(triangles.end(),
                             std::make_move_iterator(mesh.triangles.begin()),
                             std::make_move_iterator(mesh.triangles.end()));
        }

        // Calculate transformation matrix
        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::translate(trans, position);
        trans = glm::rotate(trans, glm::radians(rotation.x), glm::vec3(1, 0, 0));
        trans = glm::rotate(trans, glm::radians(rotation.y), glm::vec3(0, 1, 0));
        trans = glm::rotate(trans, glm::radians(rotation.z), glm::vec3(0, 0, 1));
        trans = glm::scale(trans, scale);
        model.transform = trans;

        models.push_back(model);

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
     * @brief Adds a keyframe to the camera's animation.
     * @param keyframe The keyframe to add.
     */
    void addCameraKeyframe(const Keyframe &keyframe)
    {
        camera.animation.addKeyframe(keyframe);
    }

    /**
     * @brief Sends scene-related data (like camera parameters) to the GPU.
     */
    void sendData()
    {
        camera.sendCameraData();
        bindTextures();
    }

    const std::vector<GLuint> &getTextureIDs() const
    {
        return textureIDs;
    }

    /**
     * @brief Binds all scene textures to OpenGL texture units.
     */
    void bindTextures() const
    {
        for (size_t i = 0; i < textureIDs.size(); ++i)
        {
            glActiveTexture(GL_TEXTURE2 + i);
            glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
        }
    }

    /**
     * @brief Updates animations for scene objects based on the elapsed time.
     * @param time The current simulation time.
     */
    void updateAnimation(float time)
    {
        camera.updateAnimation(time);

        bool sphereUpdated = false;
        for (size_t i = 0; i < spheres.size(); ++i)
        {
            if (!sphereAnimations[i].keyframes.empty())
            {
                spheres[i].center = glm::vec3(sphereAnimations[i].interpolate(time)[3]);
                sphereUpdated = true;
            }
        }
        if (sphereUpdated)
            uploadSphereBuffer();

        // bool modelUpdated = false;
        // for (size_t i = 0; i < models.size(); ++i)
        // {
        //     if (!modelAnimations[i].keyframes.empty())
        //     {
        //         models[i].transform = modelAnimations[i].interpolate(time);
        //         modelUpdated = true;
        //     }
        // }
        // if (modelUpdated)
        //     uploadBuffers();
    }
};
