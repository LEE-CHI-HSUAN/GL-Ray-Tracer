# GL Ray Tracer

![Windows](https://img.shields.io/badge/Windows-0078D6?style=flat&logo=windows&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=flat&logo=linux&logoColor=black)

A cross-platform, educational ray tracer built from scratch using **OpenGL Compute Shaders**. 

This project relies on software-defined path tracing, meaning **no dedicated hardware ray-tracing cores (like NVIDIA RT cores) are required**. It is optimized to run decently even on integrated GPUs.

> [!NOTE]
> Currently supports x86/x64 architectures; ARM is not supported.

## ✨ Features

- **Path Tracing:** Physically based rendering using compute shaders.
- **Acceleration Structure:** Bounding Volume Hierarchy (BVH) for fast ray-triangle intersections.
- **Model Support:** Wavefront `.obj` file loading.
- **Materials:** Texture mapping and smoothness handling.
- **Animation:** Keyframe animation support.

## 🚀 Quick Start

### 1. Clone the Repository

Because this project uses submodules, be sure to clone it recursively:

```bash
git clone --recurse-submodules https://github.com/LEE-CHI-HSUAN/GL-Ray-Tracer.git
cd GL-Ray-Tracer
```

> If you already cloned it normally, run `git submodule update --init`

### 2. Download Example Assets

To test the renderer, download the example assets from the [release page](https://github.com/LEE-CHI-HSUAN/GL-Ray-Tracer/releases/) and extract them into the root directory of the project.

Your folder structure should look like this:

```
[root]/
├─ asset/
│  ├─ models/
│  └─ textures/
├─ ...
└─ main.cpp
```

## 🛠️ How to Build

### Dependencies

Before building, ensure your GPU drivers are up to date. You will need the following core packages:

- FreeGLUT
- GLEW
- GLM (Optional: CMake will fetch and build this automatically if not found).

**Install via Package Managers:**

- **Windows (vcpkg):**
  ```powershell
  vcpkg install glew:x64-windows freeglut:x64-windows glm:x64-windows
  ```
- **Windows (MSYS2/Pacman):**
  ```bash
  pacman -Syu mingw-w64-x86_64-freeglut mingw-w64-x86_64-glew mingw-w64-x86_64-glm
  ```
- **Ubuntu / Debian (APT):**
  ```bash
  sudo apt update
  sudo apt install libglew-dev freeglut3-dev libglm-dev
  ```
- **macOS / Linux (Homebrew):**
  ```bash
  brew install glew freeglut glm
  ```

### Build Instructions

Once dependencies are installed, compile the project using CMake:

```bash
# Generate build files
cmake -S . -B build 

# Compile the project
cmake --build build --config Release
```

## 🎮 Running the Ray Tracer

After a successful build, the executable `RayTracer` will be located in the `build/` directory. **Ensure you run the executable from the project's root directory** so it can locate the `asset/` folder correctly.

```bash
./build/RayTracer
```

Use **WASD** to move and **arrow keys** to look around.

## 📚 Documentation

For more in-depth information about the inner workings of the engine and how to use it, please refer to the documentation:

- [Architecture Overview](doc/architecture.md)
