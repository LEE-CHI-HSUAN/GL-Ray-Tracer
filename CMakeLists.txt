# Use 3.20 to properly support C++23 standard detection
cmake_minimum_required(VERSION 3.20)
project(RayTracerProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include FetchContent for automatic dependency downloading if needed
include(FetchContent)

# Find Packages
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)

# Try finding GLM on the system first
find_package(glm CONFIG QUIET)
if(NOT glm_FOUND)
    message(STATUS "GLM not found on system. Fetching from GitHub...")
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.3
    )
    FetchContent_MakeAvailable(glm)
endif()

# Source and Header files
set(HEADERS
    renderer/RayTracer.hpp
    scene/Animation.hpp
    scene/Camera.hpp
    scene/Mesh.hpp
    scene/Scene.hpp
    scene/SceneConfig.hpp
    scene/SceneTypes.hpp
    utils/ShaderLoader.hpp
    utils/TextureLoader.hpp
)

set(SOURCES
    main.cpp
    renderer/RayTracer.cpp
    scene/SceneConfig.cpp
    utils/ShaderLoader.cpp
    utils/TextureLoader.cpp
)

# Executable
add_executable(RayTracer ${SOURCES} ${HEADERS})

# Include Directories
# Keeping the root folder and cyCodeBase. Subdirectories are resolved relative to root.
target_include_directories(RayTracer PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/submodules
)

# Link Libraries
target_link_libraries(RayTracer PRIVATE
    OpenGL::GL
    GLEW::GLEW
    GLUT::GLUT
    glm::glm
)
