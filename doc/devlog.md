# Phase 1: Basic Ray Tracer

## Primary ray generation

Ray tracing algorithm calculates a pixel color by casting a ray from the camera towards that pixel. The first step of ray tracing is to generate rays, represented by origin and direction.
The origin of the ray is the camera position. The direction is the vector from origin to a pixel, so the goal is to calculate pixel position.

### pixel position

Imagine a plane in front of the camera where rays project onto it and then form an image. The pixel positions lie on that plane. There are infinite such planes we can use to calculate ray directions if we don't constraint their length.

After I fixed the plane to a certain distance away, the only variance that can affect ray directions is the **field of view**, which is a configurable parameter of the camera.

Now that the width and height of the screen is definite, I first calculate the position of the bottom-left corner and then the offset from that corner according to screen space pixel position. The result is the corner of each pixel, so we need to offset by 0.5 to move to the center.

The direction of rays are finally aquired by subtraction and normalization. Rays are calculated in camera's local coordinate, we can multiply by its transformation matrix to get the rays in the global coordinate.

|      |      |      |
| ---- | ---- | ---- |
| ![pixel](../asset/pixelpos.png) | ![pixel](../asset/pixelposcenter.png) | ![pixel](../asset/rays.png) |

## Sphere primative calculation

A ray needs to hit an object so that the color of pixel can be calculated. The key information of a hit is the position and surface normal. Before we can render complex meshes, it is more easy to just render some simple shapes that are lightweight and easy to compute ray-object intersection. A sphere is a good start because the intersection of a line and a sphere is easy to compute.

A ray can be represented by a point plus a direction,

$$
P = \text{origin} + t\cdot\text{direction}
$$

where $t$ is an arbitrary positive integer. By determine $t$, we can find a point on the ray. So the possible 2 intersection points can be obtained by solving the following equation:

$$
\text{radius}^2 = ||\text{center} - P||^2 \\
= ||\text{center} - (\text{origin} + t\cdot\text{direction})||^2
$$

This is a quadratic equation of $t$, and there is a formula to get the 2 roots of it.

With the basic intersection detection algorithm, I am able to find the point and normal of on the surface of a sphere a ray hit. The following images are depth and normal which are rendered by ray casting.

|     |     |
| --- | --- |
| ![](../asset/ballsnormal.png) | ![](../asset/ballsdepth.png) |

## Ray tracing function
