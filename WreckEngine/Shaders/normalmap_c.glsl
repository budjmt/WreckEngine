#version 450

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform imageCube NormalTex;
layout(binding = 0) uniform samplerCube NoiseTex;

layout(location = 0) uniform float Radius;

#include height.inc
#include CubeDirection.inc

const ivec2 OFF_NONE  = ivec2( 0,  0);
const ivec2 OFF_UP    = ivec2( 0, -1);
const ivec2 OFF_DOWN  = ivec2( 0,  1);
const ivec2 OFF_LEFT  = ivec2(-1,  0);
const ivec2 OFF_RIGHT = ivec2( 1,  0);

// Gets an offset cube direction
vec3 getOffsetCubeDirection(in ivec2 dims, in ivec2 offs, in int face)
{
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy) + offs;
    vec3 dir = getCubeDirection(coords, dims, face);
    return dir;
}

// Gets a height vector
vec3 getOffsetHeightVector(in ivec2 dims, in ivec2 offs, in int face)
{
    vec3 dir = getOffsetCubeDirection(dims, offs, face);
    float noise = texture(NoiseTex, dir).r;

    // height values must be >= 0 so the normal doesn't invert
    // that's why we add radius, to ensure accuracy + that not happening for noise < 0
    return getRawHeight(dir, noise + Radius);
}

// Gets a normal from the given points representing a triangle
vec3 getTriNormal(in vec3 v0, in vec3 v1, in vec3 v2)
{
    vec3 s1 = v1 - v0;
    vec3 s2 = v2 - v0;
    return normalize(cross(s1, s2));
}

void main()
{
    ivec3 imageCoords = ivec3(gl_GlobalInvocationID);
    ivec2 coords = imageCoords.xy;
    ivec2 dims = ivec2(imageSize(NormalTex));
    int face = imageCoords.z;

#if 1
    /**
     * +--+--+
     * | /|\ |
     * |/0|0\|
     * +--+--+
     * |\0|0/|
     * | \|/ |
     * +--+--+
     * The four triangles with zeros contribute to the
     * current point's normal
     */

    // Our height values
    vec3 at = getOffsetHeightVector(dims, OFF_NONE, face);
    vec3 u  = getOffsetHeightVector(dims, OFF_UP, face);
    vec3 d  = getOffsetHeightVector(dims, OFF_DOWN, face);
    vec3 l  = getOffsetHeightVector(dims, OFF_LEFT, face);
    vec3 r  = getOffsetHeightVector(dims, OFF_RIGHT, face);

    // Average all of the face normals
    vec3 normal = vec3(0);
    normal += getTriNormal(at, u, l);
    normal += getTriNormal(at, r, u);
    normal += getTriNormal(at, d, r);
    normal += getTriNormal(at, l, d);
#else
    vec3 normal = getOffsetCubeDirection(dims, OFF_NONE, face);
    normal = texture(NoiseTex, normal).rgb;
#endif

    normal = normalize(normal);
    imageStore(NormalTex, imageCoords, vec4(normal, 0));
}
