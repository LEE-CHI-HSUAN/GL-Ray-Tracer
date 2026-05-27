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
---
config:
  flowchart:
    curve: basis
---
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
        +dispatchCompute(float time)
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
---
config:
  flowchart:
    curve: basis
---
graph LR
    subgraph Buffers ["Data Buffers (CPU to GPU)"]
        CB[CameraBlock: Uniform Buffer]
        SB[SphereBuffer: Shader Storage Buffer]
    end

    subgraph Shader ["Ray Tracing Shader (compute.glsl / ray_tracing.glsl)"]
        direction TB
        Main[main]
        GPR[getPrimaryRay]
        RCFS[rayCastForSphere]
        RSH[raySphereHit]
        Output[(img_output: image2D)]

        Main --> GPR
        Main --> RCFS
        RCFS --> RSH
        Main --> Output
    end

    CB -.- GPR
    SB -.- RCFS
    GPR -.-> Main
    RCFS -.-> Main
    
    style Buffers fill:#d9b,stroke:#333,stroke-width:0px
    style Shader fill:#bbf,stroke:#333,stroke-width:0px
    style Output fill:#dfd,stroke:#333,stroke-width:1px
    linkStyle 4,5 stroke:#999,stroke-width:2px;
```

The shader architecture follows a standard ray tracing pipeline:
1.  **Entry Point (`main`)**: Orchestrates the ray tracing process for each pixel.
2.  **Ray Generation (`getPrimaryRay`)**: Uses `CameraBlock` uniform data to transform pixel coordinates into world-space rays.
3.  **Intersection Logic (`rayCastForSphere` & `raySphereHit`)**: Iterates through the `SphereBuffer` to find the closest intersection point.
4.  **Output**: Stores the resulting color (e.g., normal mapping or depth) into the `img_output` texture.

# References

- [How to Install and Use GLUT in Visual Studio Code | Medium](https://medium.com/@aleksej.gudkov/how-to-install-and-use-glut-in-visual-studio-code-46c30243b264)
- [C++ OpenGL setup for VSCode in 2min](https://www.youtube.com/watch?v=Y4F0tI7WlDs)
- [Modern OpenGL Tutorial - Compute Shaders](https://www.youtube.com/watch?v=nF4X9BIUzx0)
- [Random directions on hemisphere](https://math.stackexchange.com/questions/1163260/random-directions-on-hemisphere-oriented-by-an-arbitrary-vector)
- [Coding Adventure: Ray Tracing](https://www.youtube.com/watch?v=Qz0KTGYJtUk)

## Tutorials

- [OpenGL Official Index](https://wikis.khronos.org/opengl/Getting_Started#Tutorials_and_How_To_Guides)
- [OpenGLBook Index](https://openglbook.com/)
- [Learn OpenGL | GLFW](https://learnopengl.com/)
- [OGL | GLUT](https://ogldev.org/)
- [Anton's OpenGL 4 Tutorials (with ray trace) | Glad | GLFW](https://antongerdelan.net/opengl/)
- [opengl-tutorial | GLFW](https://www.opengl-tutorial.org/)