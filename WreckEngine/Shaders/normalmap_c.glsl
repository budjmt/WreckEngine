#version 450

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform imageCube NoiseTex;
layout(rgba32f, binding = 1) uniform imageCube NormalTex;
layout(location = 0) uniform float Radius;

#include height.inc
#include CubeDirection.inc

const ivec2 OFF_NONE  = ivec2( 0,  0);
const ivec2 OFF_UP    = ivec2( 0, -1);
const ivec2 OFF_DOWN  = ivec2( 0,  1);
const ivec2 OFF_LEFT  = ivec2(-1,  0);
const ivec2 OFF_RIGHT = ivec2( 1,  0);

// Swaps X and Y of the given vector
void swapXY(inout ivec3 vec)
{
    int temp = vec.x;
    vec.x = vec.y;
    vec.y = temp;
}

ivec3 getOffsetCoords(in ivec2 coords, in ivec2 offset, in ivec2 dims, in int face)
{
    // oc == offset coordinates
    ivec3 oc = ivec3(coords + offset, face);
    bool swap = false;

    if (oc.x == -1)
    {
        // We need to move to the left
        switch (face)
        {
            case FACE_POSITIVE_X:
                oc.z = FACE_POSITIVE_Z;
                oc.x += dims.x;
                break;
            case FACE_NEGATIVE_X:
                oc.z = FACE_NEGATIVE_Z;
                oc.x += dims.x;
                break;
            case FACE_POSITIVE_Y:
                // Move left from top (left face)
                oc.z = FACE_NEGATIVE_X;
                oc.x += dims.x;
                swap = true; /*swapXY(oc);*/
                break;
            case FACE_NEGATIVE_Y:
                // Move left from bottom (right face)
                oc.z = FACE_POSITIVE_X;
                oc.x += dims.x;
                swap = true; /*swapXY(oc);*/
                break;
            case FACE_POSITIVE_Z:
                oc.z = FACE_NEGATIVE_X;
                oc.x += dims.x;
                break;
            case FACE_NEGATIVE_Z:
                oc.z = FACE_POSITIVE_X;
                oc.x += dims.x;
                break;
        }
    }
    else if (oc.x >= dims.x)
    {
        // We need to move to the right
        switch (face)
        {
            case FACE_POSITIVE_X:
                oc.z = FACE_NEGATIVE_Z;
                oc.x -= dims.x;
                break;
            case FACE_NEGATIVE_X:
                oc.z = FACE_POSITIVE_Z;
                oc.x -= dims.x;
                break;
            case FACE_POSITIVE_Y:
                // Move right from top (right face)
                oc.z = FACE_POSITIVE_X;
                oc.x -= dims.x;
                swap = true; /*swapXY(oc);*/
                break;
            case FACE_NEGATIVE_Y:
                // Move right from bottom (left face)
                oc.z = FACE_NEGATIVE_X;
                oc.x -= dims.x;
                swap = true; /*swapXY(oc);*/
                break;
            case FACE_POSITIVE_Z:
                oc.z = FACE_POSITIVE_X;
                oc.x -= dims.x;
                break;
            case FACE_NEGATIVE_Z:
                oc.z = FACE_NEGATIVE_X;
                oc.x -= dims.x;
                break;
        }
    }
    if (oc.y == -1)
    {
        // We need to move "up"
        switch (face)
        {
            case FACE_POSITIVE_X:
                // Move up from right (top face)
                oc.z = FACE_POSITIVE_X;
                oc.y += dims.y;
                swap = true; /*swapXY(oc);*/
                break;
            case FACE_NEGATIVE_X:
                // Move up from left (bottom face)
                oc.z = FACE_NEGATIVE_Y;
                oc.y += dims.y;
                swap = true; /*swapXY(oc);*/
                break;
            case FACE_POSITIVE_Y:
                oc.z = FACE_NEGATIVE_Z;
                oc.y += dims.y;
                break;
            case FACE_NEGATIVE_Y:
                oc.z = FACE_POSITIVE_Z;
                oc.y += dims.y;
                break;
            case FACE_POSITIVE_Z:
                oc.z = FACE_POSITIVE_Y;
                oc.y += dims.y;
                break;
            case FACE_NEGATIVE_Z:
                oc.z = FACE_NEGATIVE_Y;
                oc.y += dims.y;
                break;
        }
    }
    else if (oc.y >= dims.y)
    {
        // We need to move "down"
        switch (face)
        {
            case FACE_POSITIVE_X:
                // Move down from right (bottom face)
                oc.z = FACE_NEGATIVE_Y;
                oc.y -= dims.y;
                swap = true; /*swapXY(oc);*/
                break;
            case FACE_NEGATIVE_X:
                // Move down from left (top face)
                oc.z = FACE_POSITIVE_Y;
                oc.y -= dims.y;
                swap = true; /*swapXY(oc);*/
                break;
            case FACE_POSITIVE_Y:
                oc.z = FACE_NEGATIVE_Z;
                oc.y -= dims.y;
                break;
            case FACE_NEGATIVE_Y:
                oc.z = FACE_POSITIVE_Z;
                oc.y -= dims.y;
                break;
            case FACE_POSITIVE_Z:
                oc.z = FACE_POSITIVE_Y;
                oc.y -= dims.y;
                break;
            case FACE_NEGATIVE_Z:
                oc.z = FACE_NEGATIVE_Y;
                oc.y -= dims.y;
                break;
        }
    }
    // Z will never be "out of bounds"

    return oc;
}

vec3 getHeightVector(in ivec2 dims, in ivec2 offs, in int face)
{
    ivec2 originalCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec3 coords = getOffsetCoords(originalCoords, offs, dims, face);
    vec3 dir = normalize(getCubeDirection(coords.xy, dims, coords.z));
    vec3 pos = dir * Radius;
    float noise = imageLoad(NoiseTex, coords).r;
    // NOTE - .r won't work if we're using cellular noise!

    return getRawHeight(dir, noise);
}

vec3 getTriNormal(vec3 v0, vec3 v1, vec3 v2)
{
    vec3 s1 = v1 - v0;
    vec3 s2 = v2 - v0;
    return cross(s1, s2);
}

void main()
{
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = ivec2(imageSize(NormalTex));
    int face = int(gl_GlobalInvocationID.z);

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
    vec3 ur = getHeightVector(dims, OFF_UP + OFF_RIGHT, face);
    vec3 d  = getHeightVector(dims, OFF_DOWN, face);
    vec3 dr = getHeightVector(dims, OFF_DOWN + OFF_RIGHT, face);
    vec3 dl = getHeightVector(dims, OFF_DOWN + OFF_LEFT, face);
    vec3 l  = getHeightVector(dims, OFF_LEFT, face);
    vec3 r  = getHeightVector(dims, OFF_RIGHT, face);

    vec3 normal = vec3(0, 0, 0);
    normal += getTriNormal(ul, u, at);
    normal += getTriNormal(ul, at, l);
    normal += getTriNormal(u, r, at);
    normal += getTriNormal(u, ur, r);
    normal += getTriNormal(at, r, dr);
    normal += getTriNormal(at, dr, d);
    normal += getTriNormal(l, at, d);
    normal += getTriNormal(l, d, dl);
    normal = normalize(normal);

    if (length(normal) <= 0.001) {
        normal = vec3(0, 1, 0);
    }

    //normal = normal * 0.5 + 0.5;
    imageStore(NormalTex, ivec3(coords, face), vec4(normal, 0.0));
}
