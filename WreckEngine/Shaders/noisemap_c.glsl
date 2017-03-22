#version 450

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
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

uniform float Zoom;

void main()
{
    // "For cube maps, the size will be ivec2​; the third dimension would always be 6, so it is not returned"

    ivec3 imageCoords = ivec3(gl_GlobalInvocationID);
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = ivec2(imageSize(Tex));
    int face = int(gl_GlobalInvocationID.z);
    vec2 uv = vec2(coords.xy) / vec2(dims);

    vec3 normalizedDir = getCubeDirection(coords, dims, face);
    vec3 dir = normalizedDir * Zoom;
#if USE_CELLULAR
    vec2 n = noise3D(dir);
    vec4 color = vec4(0, n, 1.0);
#else
    float n = noise3D(dir);
    vec4 color = vec4(n, n, n, 1.0);
#endif

    imageStore(Tex, imageCoords, color);
}
