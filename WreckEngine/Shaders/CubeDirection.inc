// Same as OpenGL's face indices
#define FACE_POSITIVE_X 0
#define FACE_NEGATIVE_X 1
#define FACE_POSITIVE_Y 2
#define FACE_NEGATIVE_Y 3
#define FACE_POSITIVE_Z 4
#define FACE_NEGATIVE_Z 5

// Gets the direction used for the cubemap
vec3 getCubeDirection(in vec2 uv, in int face)
{
    // Remap UV to be in the [-1, 1] range
    uv = uv * 2.0 - 1.0;
    vec3 dir = vec3(1, 0, 0);
    float x = uv.x;
    float y = uv.y;
    switch (face)
    {
        case FACE_POSITIVE_X: dir = vec3(+1.0, y, -x); break;
        case FACE_NEGATIVE_X: dir = vec3(-1.0, y, x); break;
        case FACE_POSITIVE_Y: dir = vec3(x, +1.0, -y); break;
        case FACE_NEGATIVE_Y: dir = vec3(x, -1.0, y); break;
        case FACE_POSITIVE_Z: dir = vec3(x, y, +1.0); break;
        case FACE_NEGATIVE_Z: dir = vec3(-x, y, -1.0); break;
    }
    return normalize(dir);
}

// Gets the direction used for the cubemap
vec3 getCubeDirection(in ivec2 coords, in ivec2 dims, in int face)
{
    vec2 uv = vec2(coords) / vec2(dims);
    return getCubeDirection(uv, face);
}

// Gets the direction used for the cubemap
vec3 getCubeDirection(in ivec3 coords, in ivec2 dims)
{
    return getCubeDirection(coords.xy, dims, coords.z);
}