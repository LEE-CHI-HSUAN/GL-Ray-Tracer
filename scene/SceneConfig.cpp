#include "SceneConfig.hpp"

using namespace std::literals;

void SceneConfig::loadScene1(Scene& scene) {
    scene.createModel(
        "asset/models/cube.obj"s,
        glm::vec3(0, 0, -7),
        glm::vec3(38, -23, 14),
        glm::vec3(1, 1, 1),
        Material{.color = glm::vec4(0), .emission_color = glm::vec3(1.0, 1.0, 1.0), .emission_strength = 10.0f}
    );

    scene.createModel(
        "asset/models/cube.obj"s,
        glm::vec3(-4, 2, -11),
        glm::vec3(0, 0, 0),
        glm::vec3(1, 1, 1),
        Material{.color = glm::vec4(0.4, 0.76, 0.21, 1.0), .roughness = 0.7},
        "asset/textures/Bricks101_1K-JPG/Bricks101_1K-JPG_Color.jpg"s,
        "asset/textures/Bricks101_1K-JPG/Bricks101_1K-JPG_Roughness.jpg"s
    );

    scene.createModel(
        "asset/models/monkey.obj"s,
        glm::vec3(-4, -2, -6),
        glm::vec3(0, 0, 0),
        glm::vec3(1, 1, 1),
        Material{.color = glm::vec4(0.8, 0.21, 0.17, 1.0), .roughness = 1.0},
        "asset/textures/checker.png"s
    );

    scene.createModel(
        "asset/models/monkey_s.obj"s,
        glm::vec3(4, 0, -11),
        glm::vec3(38, -23, 14),
        glm::vec3(1, 1, 1),
        Material{.color = glm::vec4(0.9, 0.0, 0.0, 1.0), .roughness = 0.8},
        "asset/textures/Wood067_1K-JPG/Wood067_1K-JPG_Color.jpg"s,
        "asset/textures/Wood067_1K-JPG/Wood067_1K-JPG_Roughness.jpg"s
    );
}

void SceneConfig::loadScene2(Scene& scene) {
    // light
    scene.createModel(
        "asset/models/cube.obj"s,
        glm::vec3(0, 3.9, -6),
        glm::vec3(0, 0, 0),
        glm::vec3(1, 0.1, 1),
        Material{.emission_color = glm::vec3(1.0, 1.0, 1.0), .emission_strength = 30.0f, .roughness = 1.0f}
    );

    // floor
    scene.createModel(
        "asset/models/cube.obj"s,
        glm::vec3(0, -4, -6),
        glm::vec3(0, 0, 0),
        glm::vec3(5, 0.1, 6),
        Material{.color = glm::vec4(1, 1, 1, 1), .roughness = 1.0f},
        "asset/textures/checker_bw.jpg"s
    );

    // ceiling
    scene.createModel(
        "asset/models/cube.obj"s,
        glm::vec3(0, 4, -6),
        glm::vec3(0, 0, 0),
        glm::vec3(5, 0.1, 6),
        Material{.color = glm::vec4(1, 1, 1, 1), .roughness = 1.0f}
    );

    // left wall
    scene.createModel(
        "asset/models/cube.obj"s,
        glm::vec3(-5, 0, -6),
        glm::vec3(0, 0, 0),
        glm::vec3(0.1, 4, 6),
        Material{.color = glm::vec4(1, 0.5, 0.5, 1), .roughness = 1.0f}
    );

    // right wall
    scene.createModel(
        "asset/models/cube.obj"s,
        glm::vec3(5, 0, -6),
        glm::vec3(0, 0, 0),
        glm::vec3(0.1, 4, 6),
        Material{.color = glm::vec4(0.5, 0.5, 1, 1), .roughness = 1.0f}
    );

    // front wall
    scene.createModel(
        "asset/models/cube.obj"s,
        glm::vec3(0, 0, -12),
        glm::vec3(0, 0, 0),
        glm::vec3(5, 4, 0.1),
        Material{.color = glm::vec4(0.5, 1, 0.5, 1), .roughness = 1.0f},
        "asset/textures/Bricks101_1K-JPG/Bricks101_1K-JPG_Color.jpg"s,
        "asset/textures/Bricks101_1K-JPG/Bricks101_1K-JPG_Roughness.jpg"s
    );

    // back wall
    scene.createModel(
        "asset/models/cube.obj"s,
        glm::vec3(0, 0, 0.01),
        glm::vec3(0, 0, 0),
        glm::vec3(5, 4, 0.1),
        Material{.color = glm::vec4(1, 1, 1, 1), .roughness = 1.0f}
    );

    scene.createModel(
        "asset/models/monkey_s.obj"s,
        glm::vec3(-2.07, 0.9, -7.6),
        glm::vec3(-26, 36, 13),
        glm::vec3(1, 1, 1),
        Material{.color = glm::vec4(0.8, 0.21, 0.17, 1.0), .roughness = 0.9}
    );

    scene.createModel(
        "asset/models/cube.obj"s,
        glm::vec3(-1.97, -1.88, -7.25),
        glm::vec3(0, 34, 0),
        glm::vec3(1, 2, 1),
        Material{.color = glm::vec4(1.0, 1.0, 1.0, 1.0), .roughness = 1.0f}
    );

    scene.createModel(
        "asset/models/donut.obj"s,
        glm::vec3(1.6, -0.3, -6.4),
        glm::vec3(0, 34, 45),
        glm::vec3(1, 1, 1),
        Material{.color = glm::vec4(0.2, 0.8, 0.4, 1.0), .roughness = 0.2}
    );

    scene.createModel(
        "asset/models/cube.obj"s,
        glm::vec3(2.6, -2.97, -5.9),
        glm::vec3(0, 58, 0),
        glm::vec3(1, 1, 1),
        Material{.color = glm::vec4(1.0, 1.0, 1.0, 1.0), .roughness = 1.0f}
    );

    // Animation
    scene.addCameraKeyframe(Keyframe{.time = 0.0, .position = glm::vec3(0, 0, 0), .rotation = glm::vec3(0, 0, 0), .scale = glm::vec3(1, 1, 1)});
    scene.addCameraKeyframe(Keyframe{.time = 5.0, .position = glm::vec3(0, 0, 5), .rotation = glm::vec3(-45, 0, 0), .scale = glm::vec3(1, 1, 1)});
}
