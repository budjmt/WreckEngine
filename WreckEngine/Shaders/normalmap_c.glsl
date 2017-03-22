#version 450

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform imageCube NoiseTex;
layout(rgba32f, binding = 1) uniform imageCube NormalTex;
uniform vec3 CameraPosition;
uniform float Radius;

#if 1
#include height.inc
#include CubeDirection.inc
#else
vec3 getHeight(in vec3 dirFromCenter, in float height, in float camDist) {
    return vec3(0.0);
}

vec3 getCubeDirection(in ivec2 coords, in ivec2 dims, in int face) {
    return vec3(0.0);
}
#endif

const ivec2 OFF_NONE  = ivec2( 0,  0);
const ivec2 OFF_UP    = ivec2( 0, -1);
const ivec2 OFF_DOWN  = ivec2( 0,  1);
const ivec2 OFF_LEFT  = ivec2(-1,  0);
const ivec2 OFF_RIGHT = ivec2( 1,  0);

ivec3 getOffsetCoords(in ivec2 coords, in ivec2 offset, in ivec2 dims, in int face)
{
    // oc == offset coordinates
    ivec3 oc = ivec3(coords + offset, face);

    if (oc.x == -1)
    {
        // We need to move to the left
        switch (face)
        {
            case FACE_POSITIVE_X:
                oc.z = FACE_POSITIVE_Z; // Maybe negative?
                oc.x += dims.x;
                break;
            case FACE_NEGATIVE_X:
                oc.z = FACE_NEGATIVE_Z;
                oc.x += dims.x;
                break;
            case FACE_POSITIVE_Y:
                // TODO - Move left from top
                break;
            case FACE_NEGATIVE_Y:
                // TODO - Move left from bottom
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
                // TODO - Move right from top
                break;
            case FACE_NEGATIVE_Y:
                // TODO - Move right from bottom
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
                // TODO - Move up from right
                break;
            case FACE_NEGATIVE_X:
                // TODO - Move up from left
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
                oc.z = FACE_NEGATIVE_Y;
                oc.y += dims.y;
                break;
            case FACE_NEGATIVE_Z:
                oc.z = FACE_POSITIVE_Y;
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
                // TODO - Move down from right
                break;
            case FACE_NEGATIVE_X:
                // TODO - Move down from left
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
                oc.z = FACE_NEGATIVE_Y;
                oc.y -= dims.y;
                break;
            case FACE_NEGATIVE_Z:
                oc.z = FACE_POSITIVE_Y;
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
    float dist = distance(pos, CameraPosition);
    float noise = imageLoad(NoiseTex, coords).r;
    // NOTE - .r won't work if we're using cellular noise!

    return getHeight(dir, noise, dist);
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
    vec3 at  = getHeightVector(dims, OFF_NONE, face);
    vec3 u  = getHeightVector(dims, OFF_UP, face);
    vec3 ul = getHeightVector(dims, OFF_UP + OFF_LEFT, face);
    vec3 d  = getHeightVector(dims, OFF_DOWN, face);
    vec3 dr = getHeightVector(dims, OFF_DOWN + OFF_RIGHT, face);
    vec3 l  = getHeightVector(dims, OFF_LEFT, face);
    vec3 r  = getHeightVector(dims, OFF_RIGHT, face);

    vec3 normal = vec3(0, 0, 0);
    normal += getTriNormal(ul, u, at);
    normal += getTriNormal(ul, at, l);
    normal += getTriNormal(u, r, at);
    normal += getTriNormal(at, r, dr);
    normal += getTriNormal(at, dr, d);
    normal += getTriNormal(l, at, d);
    normal = normalize(normal);

    imageStore(NormalTex, ivec3(coords, face), vec4(normal, 0.0));
}
