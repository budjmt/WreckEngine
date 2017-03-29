#version 450

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// Can definitely just be R32F if we don't use cellular noise (R32G32F if so?)
layout(rgba32f, binding = 0) uniform imageCube Tex;

#define USE_SIMPLEX 0
#define USE_PERLIN 1
#define USE_CELLULAR 0

#if USE_SIMPLEX
    #include Simplex.inc
    #define noise2D simplex2D
    #define noise3D simplex3D
#elif USE_PERLIN
    #include Perlin.inc
    #define noise2D perlin2D
    #define noise3D perlin3D
#elif USE_CELLULAR
    #include Cellular.inc
    #define noise2D cellular2D
    #define noise3D cellular3D
#else
float nullNoise2D(vec2 v) { return 0.0; }
float nullNoise3D(vec3 v) { return 0.0; }
    #define noise2D nullNoise2D
    #define noise3D nullNoise3D
#endif

#include CubeDirection.inc

layout(location = 0) uniform float Zoom = 6; // Actually "octave"

// TODO - Doesn't support cellular
float getNoiseValue(in vec3 dir)
{
    // Let's mix together some octaves of noise
    float n1 = noise3D(dir * 2.0);
    float n2 = noise3D(dir * 4.0);
    float n3 = noise3D(dir * 8.0);
    float n4 = noise3D(dir * 16.0);

    float noise = (n1 * n4) + (n2 * n3) /*- (n1 * n3) + (n2 * n4)*/;
    return clamp(noise, -1.0, 1.0);
}

void main()
{
    // "For cube maps, the size will be ivec2​; the third dimension would always be 6, so it is not returned"

    ivec3 imageCoords = ivec3(gl_GlobalInvocationID);
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = ivec2(imageSize(Tex));
    int face = int(gl_GlobalInvocationID.z);
    vec2 uv = vec2(coords.xy) / vec2(dims);

    vec3 normalizedDir = getCubeDirection(coords, dims, face);
#if USE_CELLULAR
    vec2 n = noise3D(normalizedDir * Zoom);
    vec4 color = vec4(0, n, 1.0);
#else
    //float n = noise3D(normalizedDir * Zoom);
    float n = getNoiseValue(normalizedDir);
    vec4 color = vec4(n, n, n, 1.0);
#endif

    imageStore(Tex, imageCoords, color);
}
