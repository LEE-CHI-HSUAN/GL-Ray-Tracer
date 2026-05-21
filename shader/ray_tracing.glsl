#version 460 core

// Process pixels in 16x16 blocks
layout(local_size_x = 16, local_size_y = 16) in;

// The texture we are writing to
layout(rgba32f, binding = 0) uniform image2D img_output;

/* ---Struct--- */

struct Sphere
{
    vec3 center;
    float radius;
};

struct Ray
{
    vec3 origin;
    vec3 dir;
};

struct RayCastHit
{
    bool hit;
    float distance;
    vec3 point;
    vec3 normal;
};

/* ---Global Variables---*/

layout (std140) uniform CameraBlock {
    float near_clipping_plane;
    float far_clipping_plane;
    float FoV;
    float aspect_ratio;
    mat4 transform;
} cam;

layout(std430, binding=1) buffer SphereBuffer {
    int num_sphere;
    // int _padding[3];
    Sphere spheres[]; // alignment: 16
};

/* ---Helper Functions--- */

RayCastHit raySphereHit(Ray ray, Sphere sphere) {
    vec3 oc = ray.origin - sphere.center;
    
    // Simplified quadratic for normalized direction
    float b = dot(oc, ray.dir);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float h = b * b - c; // The discriminant
    
    if (h >= 0.0)  {
        h = sqrt(h);
        float t1 = -b - h;
        float t2 = -b + h;

        // Return the closest intersection in front of the camera
        float shortest_distance = -1.0;
        if (t1 >= 0.0) shortest_distance = t1;
        else if (t2 >= 0.0) shortest_distance = t2;

        if (shortest_distance >= 0.0)
        {
            vec3 hit_point = ray.origin + ray.dir * shortest_distance;
            vec3 normal = normalize(hit_point - sphere.center);
            return RayCastHit(true, shortest_distance, hit_point, normal);
        }
    }
    
    return RayCastHit(false, -1, vec3(0), vec3(0));
}

RayCastHit rayCastForSphere(Ray ray) {
    // initialize best_hit with a very large distance
    RayCastHit best_hit = {false, 1e10, vec3(0), vec3(0)};
    // loop over every object to get the closest hit
    for (int i = 0; i < num_sphere; i++) { 
        RayCastHit hit_info = raySphereHit(ray, spheres[i]);
        if (hit_info.hit && hit_info.distance < best_hit.distance) {
            best_hit = hit_info;
        }
    }
    return best_hit;
}

Ray getPrimaryRay(ivec2 pixel_id, ivec2 image_size) {
    // screen size
    float H = cam.near_clipping_plane * tan(radians(cam.FoV) / 2.0) * 2.0;
    float W = H * cam.aspect_ratio;
    vec3 botLeftCorner = vec3(-W / 2.0, -H / 2.0, -cam.near_clipping_plane);
    float dH = H / image_size.y;
    float dW = W / image_size.x;

    vec3 screen_offset = vec3(dW * (pixel_id.x + 0.5), dH * (pixel_id.y + 0.5), 0.0);
    vec3 pixel_pos = botLeftCorner + screen_offset;
    
    // Transform ray origin and pixel position to world space
    vec4 ray_o_world = cam.transform * vec4(0.0, 0.0, 0.0, 1.0);
    vec4 pixel_pos_world = cam.transform * vec4(pixel_pos, 1.0);
    
    return Ray(ray_o_world.xyz, normalize(pixel_pos_world.xyz - ray_o_world.xyz));
}

/* ---Main--- */

void main() {
    // turn them into shared?
    ivec2 pixel_id = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = imageSize(img_output);

    // Check bounds
    if (pixel_id.x >= dims.x || pixel_id.y >= dims.y) {
        return;
    }

    Ray ray = getPrimaryRay(pixel_id, dims);
    RayCastHit hit_info = rayCastForSphere(ray);
    
    vec4 pixel;
    if (hit_info.hit) {
        // normal
        vec3 local_normal = (inverse(cam.transform) * vec4(hit_info.normal, 0.0)).xyz;
        vec3 normal_color = local_normal * 0.5 + 0.5; // remap (-1, 1) to (0, 1)
        pixel = vec4(normal_color, 1.0);
        // // depth
        // float local_z = 1 + (inverse(cam.transform) * vec4(hit_info.point, 1.0)).z / 8.0;
        // pixel = vec4(local_z, local_z, local_z, 1.0);
    } else {
        pixel = vec4(0.0); // black
    }

    // Write to texture
    imageStore(img_output, pixel_id, pixel);
}