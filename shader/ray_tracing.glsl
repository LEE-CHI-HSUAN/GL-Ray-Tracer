#version 460 core

// Process pixels in 16x16 blocks
layout(local_size_x = 16, local_size_y = 16) in;

// The texture we are writing to
layout(rgba32f, binding = 0) uniform image2D imgOutput;

float raySphereDistance(vec3 ro, vec3 rd, vec3 center, float radius) {
    vec3 oc = ro - center;
    
    // Simplified quadratic for normalized direction
    float b = dot(oc, rd);
    float c = dot(oc, oc) - radius * radius;
    float h = b * b - c; // The discriminant
    
    if (h < 0.0) return -1; // No intersection
    
    h = sqrt(h);
    float t1 = -b - h;
    float t2 = -b + h;

    // Return the closest intersection in front of the origin
    if (t1 >= 0.0) return t1;
    if (t2 >= 0.0) return t2;
    
    // return -1.0;
    return -1;
}

void main() {
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = imageSize(imgOutput);

    // Check bounds
    if (pixel_coords.x >= dims.x || pixel_coords.y >= dims.y) {
        return;
    }
    
    // Camera
    float focal = 1;
    float FoV = 60.0; // angle
    float cam_aspect = float(dims.x) / float(dims.y);
    // screen
    float H = focal * tan(radians(FoV) / 2.0) * 2.0;
    float W = H * cam_aspect;
    vec3 botLeftCorner = vec3(-W / 2.0, -H / 2.0, -focal);
    float dH = H / dims.y;
    float dW = W / dims.x;
    // ray
    vec3 screen_offset = vec3(dW * (pixel_coords.x + 0.5), dH * (pixel_coords.y + 0.5), 0.0);
    vec3 pixel_pos = botLeftCorner + screen_offset;
    vec3 ray_o = vec3(0.0, 0.0, 0.0);
    vec3 ray_dir = normalize(pixel_pos - ray_o);

    // sphere
    vec3 sphere_o = vec3(0.0, 0.0, -10.0);
    float sphere_r = 5.0;

    // raycast
    float dis = raySphereDistance(ray_o, ray_dir, sphere_o, sphere_r);
    vec3 hitPoint = ray_o + ray_dir * dis;
    vec3 normal = normalize(hitPoint - sphere_o);
    vec3 normalColor = normal * 0.5 + 0.5;

    vec4 pixel = vec4(normalColor, 1.0);

    // Write to texture
    imageStore(imgOutput, pixel_coords, pixel);
}