#version 460 core

// Process pixels in 16x16 blocks
layout(local_size_x = 16, local_size_y = 16) in;

// The texture we are writing to
layout(rgba32f, binding = 0) uniform image2D imgOutput;

void main() {
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = imageSize(imgOutput);

    // Check bounds
    if (pixel_coords.x >= dims.x || pixel_coords.y >= dims.y) {
        return;
    }

    // Calculate normalized coordinates [0.0, 1.0]
    float r = float(pixel_coords.x) / float(dims.x - 1);
    float g = float(pixel_coords.y) / float(dims.y - 1);

    vec4 pixel = vec4(r, g, 0.0, 1.0);

    // Write to texture
    imageStore(imgOutput, pixel_coords, pixel);
}