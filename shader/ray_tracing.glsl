#version 460 core

// Process pixels in 16x16 blocks
layout(local_size_x = 16, local_size_y = 16) in;

// The texture we are writing to
layout(rgba32f, binding = 0) uniform image2D imgOutput;

/* ---Struct--- */

struct Camera
{
    float nearClippingPlane;
    float farClippingPlane;
    float FoV;
    float aspectRatio;
};

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
            vec3 hitPoint = ray.origin + ray.dir * shortest_distance;
            vec3 normal = normalize(hitPoint - sphere.center);
            RayCastHit hit = RayCastHit(true, shortest_distance, hitPoint, normal);
            return hit;
        }
    }
    
    RayCastHit noHit = RayCastHit(false, -1, vec3(0), vec3(0));
    return noHit;
}

/* ---Main--- */

void main() {
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = imageSize(imgOutput);

    // Check bounds
    if (pixel_coords.x >= dims.x || pixel_coords.y >= dims.y) {
        return;
    }
    
    Camera cam = {1.0, 100.0, 60.0, float(dims.x) / float(dims.y)};

    // screen
    float H = cam.nearClippingPlane * tan(radians(cam.FoV) / 2.0) * 2.0;
    float W = H * cam.aspectRatio;
    vec3 botLeftCorner = vec3(-W / 2.0, -H / 2.0, -cam.nearClippingPlane);
    float dH = H / dims.y;
    float dW = W / dims.x;

    // ray
    vec3 screen_offset = vec3(dW * (pixel_coords.x + 0.5), dH * (pixel_coords.y + 0.5), 0.0);
    vec3 pixel_pos = botLeftCorner + screen_offset;
    vec3 ray_o = vec3(0.0, 0.0, 0.0);
    Ray ray = Ray(ray_o, normalize(pixel_pos - ray_o));

    int n_sphere = 2;
    Sphere spheres[2];
    spheres[0].center = vec3(2.0, 0.0, -10.0);
    spheres[0].radius = 5.0;
    spheres[1].center = vec3(-2.0, -1.0, -7.0);
    spheres[1].radius = 2.0;

    // raycast
    // initialize bestHit with a very large distance
    RayCastHit bestHit = {false, 1e10, vec3(0), vec3(0)};
    // loop over every object to get the closest hit
    for (int i = 0; i < n_sphere; i++) { 
        RayCastHit hitInfo = raySphereHit(ray, spheres[i]);
        if (hitInfo.hit && hitInfo.distance < bestHit.distance) {
            bestHit = hitInfo;
        }
    }

    vec4 pixel;
    if (bestHit.hit) {
        // remap normal (-1, 1) to color (0, 1)
        vec3 normalColor = bestHit.normal * 0.5 + 0.5;
        pixel = vec4(normalColor, 1.0);
    } else {
        pixel = vec4(0.0); // black
    }

    // Write to texture
    imageStore(imgOutput, pixel_coords, pixel);
}