#version 450

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// Can definitely just be R32F if we don't use cellular noise (R32G32F if so?)
layout(rgba32f, binding = 0) uniform imageCube Tex;

#include Perlin.inc
#include CubeDirection.inc

//float perlin3D(in vec3 a) { return 0.0; }
//vec3 getCubeDirection(in vec2 a, in int b) { return vec3(0.0); }

#define noise2D perlin2D
#define noise3D perlin3D

float getNoiseValue(in vec3 dir)
{
    // Let's mix together some octaves of noise
    float n1 = noise3D(dir * 1.0);
    float n2 = noise3D(dir * 2.0);
    float n3 = noise3D(dir * 4.0);
    float n4 = noise3D(dir * 8.0);

    float noise = (n1 * n4) + (n2 * n3);
    return clamp(noise, -1.0, 1.0);
}

void main()
{
    // "For cube maps, the size will be ivec2​; the third dimension would always be 6, so it is not returned"

    ivec3 imageCoords = ivec3(gl_GlobalInvocationID);
    ivec2 dims = ivec2(imageSize(Tex));
    int face = imageCoords.z;
    vec2 uv = vec2(imageCoords.xy) / vec2(dims);

    vec3 dir = getCubeDirection(uv, face);
    float n = getNoiseValue(dir) * 0.5 + 0.5;
    vec4 color = vec4(n, n, n, 1.0);

    imageStore(Tex, imageCoords, color);
}
