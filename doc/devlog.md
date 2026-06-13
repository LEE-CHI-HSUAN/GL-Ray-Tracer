# CG2026 Final Project - Path Tracing

R14922194 資工碩一 李季軒

[LEE-CHI-HSUAN/GL-Ray-Tracer: OpenGL-based path tracing algorithm.](https://github.com/LEE-CHI-HSUAN/GL-Ray-Tracer)

## Primary Ray Generation

A ray tracing algorithm calculates a pixel's color by casting a ray from the camera into the scene. The first step is generating these rays, each represented by an origin and a direction. The origin of the ray corresponds to the camera's position. The direction is the vector pointing from this origin to a specific pixel on the projection plane, making the calculation of the pixel's position the initial goal.

### Pixel Position

Imagine a plane in front of the camera where projected rays form an image. The pixel positions lie on this plane. If we do not constrain the ray lengths, there are infinitely many such planes we can use to calculate ray directions.

Once the plane is fixed at a set distance, the primary variable affecting ray directions is the camera's configurable **field of view** (FOV).

Given a fixed screen width and height, we first calculate the position of the bottom-left corner of the image plane. We then determine the offset from that corner based on the pixel's screen-space coordinates. Because this initial calculation yields the corner of each pixel, we apply an offset of 0.5 to target the pixel's center.

The final ray directions are obtained by subtraction and normalization. Because these rays are initially calculated in the camera's local coordinate system, we multiply them by the camera's transformation matrix to convert them into global coordinates.

|                                        |                                              |                                    |
| -------------------------------------- | -------------------------------------------- | ---------------------------------- |
| ![pixel](../asset/images/pixelpos.png) | ![pixel](../asset/images/pixelposcenter.png) | ![pixel](../asset/images/rays.png) |

## Sphere Primitive Calculation

For a pixel color to be calculated, its corresponding ray must intersect an object. The key details of this intersection are the hit position and the surface normal. Before rendering complex meshes, it is helpful to render simpler shapes that are lightweight and computationally straightforward. A sphere is a suitable starting point because line-sphere intersections are relatively easy to compute.

A ray can be represented by a point plus a direction vector:

$$
P = \text{origin} + t\cdot\text{direction}
$$

where $t$ is a positive scalar. By solving for $t$, we can find points along the ray. The potential intersection points (up to two) are obtained by solving the following quadratic equation:

$$
\text{radius}^2 = ||\text{center} - P||^2 \\
= ||\text{center} - (\text{origin} + t\cdot\text{direction})||^2
$$

This equation can be solved using the quadratic formula to find the roots for $t$.

Using this basic intersection algorithm, we can determine the hit point and surface normal on the sphere. The following images display the depth and normal passes rendered via ray casting.

|                                      |                                     |
| ------------------------------------ | ----------------------------------- |
| ![](../asset/images/ballsnormal.png) | ![](../asset/images/ballsdepth.png) |

### Side Track: Moving Camera

To evaluate the scene from different angles, a camera fly-through system was implemented. The camera's transformation matrix and viewport parameters are synchronized with the GPU via a Uniform Buffer Object (UBO).

### Side Track: SSBO

The sphere scene data was originally hardcoded within the compute shader. It has since been moved to the CPU side and is transmitted to the shader program using a Shader Storage Buffer Object (SSBO). This change enables flexible scene updates from the CPU and establishes a foundation for loading 3D model data.

## Ray Tracing Function

Once the sphere geometry can be queried via ray casting, we can implement a basic ray tracing function to calculate the ray path and its accumulated light.

Assuming a smooth, mirror-like surface simplifies the calculation, as we only need to evaluate specular reflections rather than scattered rays. This makes the path of a ray deterministic. When a ray hits a surface, a reflected ray is generated with the hit point as its origin and the direction calculated as:

$$
\text{reflect\_dir} = \text{ray\_dir} - 2 \cdot \text{normal} \cdot (\text{normal} \cdot \text{ray\_dir})
$$

![](../asset/images/reflect_illustration.png)

When a ray hits a surface, the path throughput is multiplied by the surface color. When the ray intersects a light source, the accumulated throughput is multiplied by the light's color and intensity, and then added to the final pixel color.

The test scene contains four spheres, one of which is emissive and acts as the primary light source. A dim ambient environment light is also included to make the silhouettes more visible. A parameter controls the maximum reflection depth (bounces). With a reflection limit of 1, only the primary light source is visible in reflections. Increasing the limit to 2 allows spheres to reflect one another, and higher limits yield deeper recursive reflections at the cost of increased computation.

| #reflection=1               | #reflection=2               | #reflection=5               |
| --------------------------- | --------------------------- | --------------------------- |
| ![](../asset/images/r1.png) | ![](../asset/images/r2.png) | ![](../asset/images/r5.png) |

## Diffuse Reflection

Real-world objects rarely have perfectly smooth surfaces. To improve realism, the next step is expanding the ray tracer to handle diffuse reflection. This is done by replacing the deterministic reflection model with a random direction generator.

### Random Direction on a Hemisphere

Since standard shader languages lack a built-in random number generator, a pseudo-random generator was implemented using bit-shifting operations. This generator samples a floating-point value in the range $[0, 1]$ from a uniform distribution.

Using these random values, several methods can be used to generate a random direction.

The most straightforward method is to sample three independent floating-point values and normalize the resulting vector. However, because this samples from a unit cube, the points tend to cluster toward the corners. This produces an uneven distribution, as shown in the image on the left.

A common workaround is to discard any sampled points that fall outside the unit sphere, which yields a uniform distribution on the sphere's surface. The drawback to this rejection sampling approach is that it requires a loop to resample until a valid point is found, though the average number of iterations remains low (fewer than two).

Alternatively, one could sample floats from a normal distribution, but this is computationally more complex than uniform sampling.

Instead, the implementation adopts a formula that samples and rescales two floats to compute the spherical coordinates of a point. This approach avoids some of the more expensive trigonometric operations associated with other methods:

$$
\begin{cases} \theta = \cos^{-1}(\varepsilon_0) \\
\phi = 2\pi\varepsilon_1 \end{cases}
$$

| More points on the edges            | Uniform distribution                |
| ----------------------------------- | ----------------------------------- |
| ![](../asset/images/sampledir1.png) | ![](../asset/images/sampledir2.png) |

To generate directions over a hemisphere rather than an entire sphere, we evaluate whether the generated vector points outward relative to the surface normal using a dot product. If the result is negative, the direction vector is inverted.

#### Cosine-Weighted Distribution

The angle between the incoming ray and the surface normal affects the received energy per unit area, which is physically modeled by a cosine term. To avoid calculating this cosine factor explicitly for every sample, we can sample ray directions with a probability density proportional to $\cos(\theta)$. This allows us to average the samples directly without multiplying by $\cos(\theta)$. A simple approximation for this cosine-weighted direction is:

```glsl
sample_direction = normalize(surface_normal + random_unit_direction);
```

| Uniform                             | Cosine Weighted                     |
| ----------------------------------- | ----------------------------------- |
| ![](../asset/images/sampledir3.jpg) | ![](../asset/images/sampledir4.jpg) |

### Noise

Initial results of diffuse reflection often display significant noise. Unlike smooth surfaces where light travels along deterministic paths, diffuse surfaces scatter rays stochastically.

![](../asset/images/scatter1.png)

According to the rendering equation, evaluating the integral of incoming light requires a very high number of samples. In practice, we must set a limit on the number of samples per pixel.

As the sample count increases, the stochastic noise gradually diminishes, and the image converges toward a cleaner result. The reduction of artifacts can be observed at higher sampling densities:

| 4 samples                           | 16 samples                            |
| ----------------------------------- | ------------------------------------- |
| ![](../asset/images/scatter4.png)   | ![](../asset/images/scatter16.png)    |
| 196 samples                         | 10000 samples                         |
| ![](../asset/images/scatter196.png) | ![](../asset/images/scatter10000.png) |

Even with a simple scene, computing 100 samples per pixel per frame takes approximately 58 ms on a modern GPU (roughly 17.2 FPS), and the image remains noticeably noisy. While accumulating more samples over time improves visual quality, raw path tracing at high sample counts is challenging to achieve in real-time without optimization or denoising.

## Rendering Meshes

The initial work with spheres established the foundational ray tracing framework, making it possible to extend the renderer to support complex polygonal meshes.

### Data Structure

A 3D model is composed of a list of triangles. The most direct approach for calculating intersections is to loop through every model and test the ray against every triangle. To enable the GPU to iterate over these triangles, the scene models are loaded on the CPU, packed into a continuous array, and uploaded to the GPU.

However, multiple models in a scene often share the same underlying mesh. Storing duplicate triangle data for each instance is memory inefficient. To address this, we decouple the model instance data from the mesh data into two separate arrays. The triangle array stores unique mesh data in local space. The model array stores instance-specific information, including its transformation matrix and an offset pointing to the corresponding mesh.

![](../asset/images/modelarray1.jpg)

Because the mesh data is in local space, we must apply the model's transformation matrix to obtain world-space coordinates. While this is similar to the operations performed in a standard vertex shader, doing this per-triangle in a ray tracer scales poorly with pixel and vertex count. Alternatively, we can transform the incoming ray into the model's local space. This allows us to perform the inverse transformation only once per ray before testing intersections.

Without further optimization, this brute-force approach results in the following performance for a test scene:

- **Frame Time:** 1933 ms
- **Scene Triangle Count:** 1960

![](../asset/images/rendermesh.jpg)

### Optimization: AABB

An immediate optimization requiring minimal code modification is the ray-AABB (Axis-Aligned Bounding Box) test. By checking whether a ray intersects the bounding box of a model first, we can skip individual ray-triangle tests for models that are not hit, significantly speeding up the rendering process.

![](../asset/images/boundingbox.jpg)

This optimization reduces the frame time to 244 ms—roughly 12% of the original rendering time.

## Mixed Specular and Diffuse Reflection

![](../asset/images/light_surface.jpg)

As scene geometry becomes more complex, the shading model must be updated to produce more sophisticated images. Focusing on non-metallic materials, the previous reflection and diffuse reflection techniques can be combined to implement a material shader with both specular and diffuse components. The materials are defined by properties such as base color, emission color, emission strength, and roughness. In path tracing, any emissive surface can act as a light source, removing the need for dedicated light types.

The resulting renders are shown below:

|                                    |                                    |      |
| ---------------------------------- | ---------------------------------- | ---- |
| ![](../asset/images/specular0.jpg) | ![](../asset/images/specular1.jpg) |      |

Before further increasing visual complexity, optimization is needed to manage the GPU's computational load. The next step addresses performance by introducing a Bounding Volume Hierarchy (BVH).

## Bounding Volume Hierarchy

A BVH is a tree-like structure that hierarchically subdivides spatial elements into smaller groups. Hierarchical acceleration structures are widely used to reduce query times, with several common variations such as octrees and Binary Space Partitioning (BSP) trees. The bounding volumes can use various shapes; here, Axis-Aligned Bounding Boxes (AABBs) are used due to their simplicity and low computational cost.

Using AABBs as the volume representation integrates well with the existing intersection logic.

### Memory Layout

While constructing a binary tree is straightforward on the CPU, the memory layout requires careful consideration because the BVH must be traversed efficiently on the GPU.

GPUs perform best with contiguous memory access. Standard CPU tree implementations often use dynamically allocated nodes linked by pointers, which scatters data in memory and is difficult to traverse on the GPU. To resolve this, the BVH is constructed using a flat, array-based layout where child nodes are referenced by array indices rather than pointers.

![](../asset/images/build_tree.jpg)

This is similar to how a heap is represented within a flat array. The tree nodes are allocated within a pre-allocated contiguous array; when a new node is created, it is appended to the list, and children are linked via their array indices.

Building the BVH is a divide-and-conquer process conceptually similar to quicksort. A parent node containing a range of triangles is partitioned into two child nodes along the spatial axis ($X$, $Y$, or $Z$) that provides the best split. This process recursively subdivides the geometry until a leaf node condition is met.

### Traversing the BVH

Since GPUs generally do not support or perform well with dynamic recursion, an iterative traversal approach is used. A fixed-size local stack of 30 integers is allocated within the shader to manage the traversal state without requiring recursive function calls.

```glsl
int stack[30];
int stack_ptr = 0;
stack[stack_ptr++] = model.nodeOffset + 1; // Start from root

while(stack_ptr > 0) {
    int node_id = stack[--stack_ptr];
    // ... test node ...
    // ... push children ...
}
```

This stack-based approach manages the traversal efficiently within the GPU's registers.

- **Frame Time:** 99 ms

### AABB Culling and Children Ordering

At each node during traversal, the ray is tested against the node's AABB. If the ray misses the bounding box, the entire subtree is skipped, avoiding redundant intersection checks.

Additionally, performance can be improved by sorting child nodes based on proximity. By evaluating the intersection distance of both child AABBs, the closer child is pushed onto the stack last, ensuring it is traversed first. Finding closer intersections early allows us to cull more distant branches sooner based on the current closest hit distance $t$.

- **Frame Time:** 73 ms

### Complex Scene Test

The optimized BVH allows the renderer to handle more detailed scenes:

![](../asset/images/demo_10000.png)

## Key Challenges

- **Memory Layout:** Structuring spatial acceleration trees into flat, continuous GPU arrays to ensure optimal memory alignment and cache access.
- **Self-Intersection Bugs:** Debugging visual artifacts (surface acne) caused by floating-point precision issues, resolved by pushing ray origins slightly along the surface normal.
- **Temporal Progressive Rendering:** Mitigating variance and fireflies in progressive rendering when the scene is dominated by small, highly concentrated light sources.