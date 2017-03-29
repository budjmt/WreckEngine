#version 450

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform imageCube NormalTex;
layout(location = 0, binding = 0) uniform samplerCube NoiseTex;
layout(location = 1) uniform float Radius;

#include height.inc
#include CubeDirection.inc

const ivec2 OFF_NONE  = ivec2( 0,  0);
const ivec2 OFF_UP    = ivec2( 0, -1);
const ivec2 OFF_DOWN  = ivec2( 0,  1);
const ivec2 OFF_LEFT  = ivec2(-1,  0);
const ivec2 OFF_RIGHT = ivec2( 1,  0);

// Gets a height direction
vec3 getHeightDirection(in ivec2 dims, in ivec2 offs, in int face)
{
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy) + offs;
    vec3 dir = getCubeDirection(coords, dims, face);
    return dir;
}

// Gets a height vector
vec3 getHeightVector(in ivec2 dims, in ivec2 offs, in int face)
{
    vec3 dir = getHeightDirection(dims, offs, face);
    float noise = texture(NoiseTex, dir).r;
    // NOTE - .r won't work if we're using cellular noise!

    return getRawHeight(dir, noise);
}

// Gets a normal from the given points representing a triangle
vec3 getTriNormal(vec3 v0, vec3 v1, vec3 v2)
{
    vec3 s1 = v1 - v0;
    vec3 s2 = v2 - v0;
    return normalize(cross(s1, s2));
}

void main()
{
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = ivec2(imageSize(NormalTex));
    int face = int(gl_GlobalInvocationID.z);

#if 0
    /**
     * +--+--+
     * |\*|\ |
     * |*\|*\|
     * +--+--+
     * |\*|\*|
     * | \|*\|
     * +--+--+
     * The six triangles with stars contribute to the
     * current point's normal
     */

    // Our height values
    vec3 at = getHeightVector(dims, OFF_NONE, face);
    vec3 u  = getHeightVector(dims, OFF_UP, face);
    vec3 ul = getHeightVector(dims, OFF_UP + OFF_LEFT, face);
    vec3 d  = getHeightVector(dims, OFF_DOWN, face);
    vec3 dr = getHeightVector(dims, OFF_DOWN + OFF_RIGHT, face);
    vec3 l  = getHeightVector(dims, OFF_LEFT, face);
    vec3 r  = getHeightVector(dims, OFF_RIGHT, face);

    // Average all of the face normals
    vec3 normal = vec3(0, 0, 0);
    normal += getTriNormal(ul, u, at);
    normal += getTriNormal(ul, at, l);
    normal += getTriNormal(u, r, at);
    normal += getTriNormal(at, r, dr);
    normal += getTriNormal(at, dr, d);
    normal += getTriNormal(l, at, d);
    normal = normalize(normal);

    // Flip the Y value because apparently that's wrong
    normal.y = -normal.y;
#else
    vec3 normal = getHeightDirection(dims, OFF_NONE, face);
#endif

    imageStore(NormalTex, ivec3(coords, face), vec4(normal, 0.0));
}
