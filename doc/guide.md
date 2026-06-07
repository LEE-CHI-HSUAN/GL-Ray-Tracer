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

### Overview

```mermaid
graph LR
    subgraph Cpp ["C++ Application (CPU)"]
        direction TB
        Main[main.cpp]
        RT[RayTracer]
        S[Scene]
        Cam[Camera]
        
        Main -->|Manages| RT
        Main -->|Manages| S
        S -->|Composes| Cam
    end

    subgraph Gpu ["GPU Resources (VRAM)"]
        direction TB
        UBOP[Parameter Buffer: GL_UNIFORM_BUFFER]
        UBOC[Camera Buffer: GL_UNIFORM_BUFFER]
        SSBOS[Sphere Buffer: GL_SHADER_STORAGE_BUFFER]
        SSBOM[Model Buffer: GL_SHADER_STORAGE_BUFFER]
        SSBOT[Triangle Buffer: GL_SHADER_STORAGE_BUFFER]
        SSBON[Node Buffer: GL_SHADER_STORAGE_BUFFER]
        Tex[Accumulation Textures: Double Buffered]
        FBO[Rescale Framebuffer: FBO]
    end

    Cam -.-> |Uploads CameraData| UBOC
    S -.-> |Uploads Spheres| SSBOS
    S -.-> |Uploads Models| SSBOM
    S -.-> |Uploads Triangles| SSBOT
    S -.-> |Uploads Nodes| SSBON
    RT -.-> |Uploads RenderParams| UBOP
    RT -.-> |Dispatches Compute to| Tex
    RT -.-> |Blits Final Render| FBO
    CS[Compute Shader]
    Tex <-.->|Read/Write| CS
    UBOC -.-> |Reads| CS
    SSBOS -.-> |Reads| CS
    SSBOM -.-> |Reads| CS
    SSBOT -.-> |Reads| CS
    SSBON -.-> |Reads| CS
    UBOP -.-> |Reads| CS

    style Cpp fill:#bbf,stroke:#333,stroke-width:0px
    style Gpu fill:#d9b,stroke:#333,stroke-width:0px
    style Tex fill:#dfd,stroke:#333,stroke-width:1px
```

### C++ Side

```mermaid
classDiagram
    class RayTracer {
        -GLuint computeProgram
        -GLint workGroupSize[3]
        -GLuint textures[2]
        -int currentTexture
        -GLuint fboRescale
        -int windowWidth
        -int windowHeight
        -GLuint uboParameters
        -RenderParameters parameters
        -int max_samples
        -initTexture() void
        -initParameterBuffer() void
        -sendRenderParameters(float time) void
        +RayTracer(const string &filePath)
        +RayTracer(const string &filePath, const int windowWidth, const int windowHeight)
        +dispatchCompute(float time) int
        +setWindowSize(int width, int height) RayTracer*
        +displayScreen() void
        +ResetRenderSpp() void
        +getShaderProgram() GLuint
    }

    class RenderParameters {
        +float time
        +int samplePerPixel
        +int cumulative_samples
    }

    class Scene {
        -Camera camera
        -GLuint sphereSsbo
        -GLuint modelSsbo
        -GLuint nodeSsbo
        -GLuint triangleSsbo
        -vector~Triangle~ triangles
        -vector~Model~ models
        -vector~BVHNode~ nodes
        -spawnSpheres() void
        -uploadBuffers() void
        +Scene(const GLuint shaderProgram)
        +createModel(string filename, vec3 pos, vec3 rot, vec3 scale, Material mat) void
        +setCameraAspectRatio(int w, int h) void
        +moveCamera(vec3 direction) void
        +rotateCamera(vec2 yawPitch) void
        +sendData() void
    }

    class Camera {
        -CameraData camData
        -GLuint uboCamera
        -vec3 position
        -quat orientation
        -vec3 front
        -vec3 right
        -vec3 up
        -float movementSpeed
        -float rotationSpeed
        -updateTransformMatrix() void
        +Camera(const GLuint shaderProgram)
        +setAspect(int w, int h) void
        +move(vec3 input) void
        +rotate(vec2 input) void
        +sendCameraData() void
    }

    class CameraData {
        +float nearClippingPlane
        +float farClippingPlane
        +float FoV
        +float aspectRatio
        +mat4 transform
    }

    class Material {
        +vec4 color
        +vec3 emission_color
        +float emission_strength
        +float roughness
    }

    class Sphere {
        +vec3 center
        +float radius
        +Material material
        +Sphere()
        +Sphere(vec3 center, float radius, Material material)
    }

    class AABB {
        +vec3 min
        +vec3 max
    }

    class Model {
        +mat4 transform
        +int32_t start
        +int32_t num_faces
        +Material material
    }

    class Triangle {
        +vec4 vertex[3]
        +vec4 normal[3]
        +vec2 uv[3]
    }

    class BVHNode {
        +int childL
        +int childR
        +int elementOffset
        +int elementNum
        +AABB boundingBox
    }

    RayTracer *-- RenderParameters : composition
    Scene *-- Camera : composition
    Camera *-- CameraData : composition
    Scene ..> RayTracer : gets shaderProgram from
    Scene ..> Sphere : spawns
    Scene ..> Model : manages
    Scene ..> Triangle : manages
    Scene ..> BVHNode : manages
    Sphere *-- Material : composition
    Model *-- Material : composition
    BVHNode *-- AABB : composition
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
        MB[ModelBuffer: Shader Storage Buffer]
        TB[TriangleBuffer: Shader Storage Buffer]
        NB[NodeBuffer: Shader Storage Buffer]
    end

    subgraph Shader ["Ray Tracing Shader (compute.glsl / ray_tracing.glsl)"]
        direction TB
        Main[main]
        GPR[getPrimaryRay]
        RCFS[rayCastForSphere]
        RSH[raySphereHit]
        Output[(img_output: image2D)]
        RCFM[rayCastForModel]
        RTH[rayTriangleHit]
        RBH[rayBVHHit]
        
        Main --> GPR
        Main --> RCFS
        RCFS --> RSH
        Main --> RCFM
        RCFM --> RBH
        RBH --> RTH
        Main --> Output
    end

    CB -.-> |Read Access| GPR
    SB -.-> |Read Access| RCFS
    MB -.-> |Read Access| RCFM
    TB -.-> |Read Access| RTH
    NB -.-> |Read Access| RBH
    GPR -.-> Main
    RCFS -.-> Main
    RCFM -.-> Main
    RBH -.-> RCFM
    RTH -.-> RBH
    
    style Buffers fill:#d9b,stroke:#333,stroke-width:0px
    style Shader fill:#bbf,stroke:#333,stroke-width:0px
    style Output fill:#dfd,stroke:#333,stroke-width:1px
```

The shader architecture follows a standard ray tracing pipeline:
1.  **Entry Point (`main`)**: Orchestrates the ray tracing process for each pixel.
2.  **Ray Generation (`getPrimaryRay`)**: Uses `CameraBlock` uniform data to transform pixel coordinates into world-space rays.
3.  **Intersection Logic (`rayCastForSphere` & `raySphereHit`)**: Iterates through the `SphereBuffer` to find the closest intersection point.
4.  **Model Intersection (`rayCastForModel`, `rayBVHHit`, & `rayTriangleHit`)**: Traverses the Bounding Volume Hierarchy (BVH) stored in `NodeBuffer` and `TriangleBuffer` to efficiently find mesh geometry intersections.
5.  **Output**: Stores the resulting color (e.g., normal mapping or depth) into the `img_output` texture.
# References

- [How to Install and Use GLUT in Visual Studio Code | Medium](https://medium.com/@aleksej.gudkov/how-to-install-and-use-glut-in-visual-studio-code-46c30243b264)
- [C++ OpenGL setup for VSCode in 2min](https://www.youtube.com/watch?v=Y4F0tI7WlDs)
- [Modern OpenGL Tutorial - Compute Shaders](https://www.youtube.com/watch?v=nF4X9BIUzx0)
- [Random directions on hemisphere](https://math.stackexchange.com/questions/1163260/random-directions-on-hemisphere-oriented-by-an-arbitrary-vector)
- [Coding Adventure: Ray Tracing](https://www.youtube.com/watch?v=Qz0KTGYJtUk)
- [Sampling the hemisphere](https://ameye.dev/notes/sampling-the-hemisphere/)
- [Cosine-weighted-sampling](https://pema.dev/obsidian/math/light-transport/cosine-weighted-sampling.html)
- Real-Time Rendering, Third Edition, Tomas Akenine-Moller, Eric Haines, Naty Hoffman

## Tutorials

- [OpenGL Official Index](https://wikis.khronos.org/opengl/Getting_Started#Tutorials_and_How_To_Guides)
- [OpenGLBook Index](https://openglbook.com/)
- [Learn OpenGL | GLFW](https://learnopengl.com/)
- [OGL | GLUT](https://ogldev.org/)
- [Anton's OpenGL 4 Tutorials (with ray trace) | Glad | GLFW](https://antongerdelan.net/opengl/)
- [opengl-tutorial | GLFW](https://www.opengl-tutorial.org/)
