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

// Same as OpenGL's face indices
#define FACE_POSITIVE_X 0
#define FACE_NEGATIVE_X 1
#define FACE_POSITIVE_Y 2
#define FACE_NEGATIVE_Y 3
#define FACE_POSITIVE_Z 4
#define FACE_NEGATIVE_Z 5

uniform float Time;
uniform float Zoom;

// Gets the direction used for the cubemap
vec3 getCubeDirection(in vec2 uv)
{
    vec3 dir = vec3(1, 0, 0);
    float x = uv.x;
    float y = uv.y;
    switch (gl_GlobalInvocationID.z)
    {
        case FACE_POSITIVE_X: dir = vec3(+1.0, y, -x); break;
        case FACE_NEGATIVE_X: dir = vec3(-1.0, y, x); break;
        case FACE_POSITIVE_Y: dir = vec3(x, -1.0, y); break;
        case FACE_NEGATIVE_Y: dir = vec3(x, +1.0, -y); break;
        case FACE_POSITIVE_Z: dir = vec3(x, y, +1.0); break;
        case FACE_NEGATIVE_Z: dir = vec3(-x, y, -1.0); break;
    }
    return normalize(dir);
}

void main()
{
    // "For cube maps, the size will be ivec2​; the third dimension would always be 6, so it is not returned"

    ivec3 coords = ivec3(gl_GlobalInvocationID);
    vec2 dims = imageSize(Tex);
    vec2 uv = vec2(coords.xy) / dims;

    vec3 normalizedDir = getCubeDirection(uv * 2.0 - 1.0);
    vec3 dir = normalizedDir * Zoom;
#if USE_CELLULAR
    vec2 n = noise3D(dir);
    vec4 color = vec4(0, n, 1.0);
#else
    float n = clamp(noise3D(dir), 0., 1.);
    vec4 color = vec4(n, n, n, 1.0);
#endif

    imageStore(Tex, coords, color);
}
