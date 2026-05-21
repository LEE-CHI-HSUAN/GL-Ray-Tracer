# Development Environment Setup

- OS: Windows 11
- C++ compiler: Msys2 mingw-w64

## Install Packages

In the MinGW-w64 terminal, run:

```shell
pacman -S mingw-w64-x86_64-freeglut
pacman -S mingw-w64-x86_64-glew
```

## Compile

```shell
g++ main.cpp -o main.exe \
  -I 'C:/msys64/mingw64/include' \
  -lfreeglut -lglu32 -lopengl32 -lglew32 \
```

# Roadmap

```mermaid
flowchart TD

    cs[compute_shader]
    rd[ray_direction]
    cp[circle_primative]
    ci[circle_intersection]
    tfb[trace_function_beta]
    tf[trace_function]
    tp[triangle_primative]
    ti[triangle_intersection]
    bvh[BVH]
    is[importance_sampling]
    rt[refraction_in_trace]
    
    cs --> rd --> tfb --> tf --> bvh --> is
    cp --> ci --> tfb
    tp --> ti --> tf
    tf --> rt
    ci --> ti 
```

## Program Architecture

### C++ Side

```mermaid
classDiagram
    class RayTracer {
        -GLuint computeProgram
        -GLuint textureOutput
        -GLuint fboRescale
        +dispatchCompute()
        +displayScreen()
        +setWindowSize(int w, int h)
        +getShaderProgram() GLuint
    }

    class Scene {
        -Camera camera
        +moveCamera(vec3 direction)
        +rotateCamera(vec2 yawpitch)
        +setCameraAspectRatio(int w, int h)
        +sendData()
    }

    class Camera {
        -CameraData camData
        -GLuint uboCamera
        +move(vec3 input)
        +rotate(vec2 input)
        +setAspect(int w, int h)
        +sendCameraData()
    }

    class CameraData {
        <<struct>>
        +float nearClippingPlane
        +float farClippingPlane
        +float FoV
        +float aspectRatio
        +mat4 transform
    }

    class ShaderLoader {
        <<utility>>
        +initComputeShader(string file) GLuint
    }

    RayTracer ..> ShaderLoader : uses
    Scene *-- Camera : composition
    Camera *-- CameraData : composition
```

### Shader Side

```mermaid
flowchart TD

    main[main: init]
```


# References

- [How to Install and Use GLUT in Visual Studio Code | Medium](https://medium.com/@aleksej.gudkov/how-to-install-and-use-glut-in-visual-studio-code-46c30243b264)
- [C++ OpenGL setup for VSCode in 2min](https://www.youtube.com/watch?v=Y4F0tI7WlDs)
- [Modern OpenGL Tutorial - Compute Shaders](https://www.youtube.com/watch?v=nF4X9BIUzx0)

## Tutorials

- [OpenGL Official Index](https://wikis.khronos.org/opengl/Getting_Started#Tutorials_and_How_To_Guides)
- [OpenGLBook Index](https://openglbook.com/)
- [Learn OpenGL | GLFW](https://learnopengl.com/)
- [OGL | GLUT](https://ogldev.org/)
- [Anton's OpenGL 4 Tutorials (with ray trace) | Glad | GLFW](https://antongerdelan.net/opengl/)
- [opengl-tutorial | GLFW](https://www.opengl-tutorial.org/)