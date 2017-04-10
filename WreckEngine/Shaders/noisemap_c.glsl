#version 450

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// Can definitely just be R32F if we don't use cellular noise (R32G32F if so?)
layout(rgba32f, binding = 0) uniform imageCube Tex;

//#include Cellular.inc
#include Perlin.inc
#include Simplex.inc
#include CubeDirection.inc

float getNoiseValue(in vec3 dir)
{
#if 1
    float p1 = perlin3D(dir);
    float p2 = perlin3D(dir * 2.0);
    float p3 = perlin3D(dir * 4.0);
    float p4 = perlin3D(dir * 8.0);

    float s1 = simplex3D(dir);
    float s2 = simplex3D(dir * 2.0);
    float s3 = simplex3D(dir * 4.0);
    float s4 = simplex3D(dir * 8.0);

    float noise = (s1 * s1 * p1 * s4);// - (s1 * p3) - (s3 * p1) + (p4 * s1);
    noise -= s1 * p3 + s3 * p1;
    noise += p4 * s1 + s1l
    noise += s1;
    noise *= p1;
#else
    // Let's mix together some octaves of noise
    float n1 = perlin3D(dir * 1.0);
    float n2 = perlin3D(dir * 2.0);
    float n3 = perlin3D(dir * 4.0);
    float n4 = perlin3D(dir * 8.0);
    float noise = (n1 * n4) + (n2 * n3);
#endif
    return clamp(noise, -1.0, 1.0);
}

void main()
{
    // "For cube maps, the size will be ivec2​; the third dimension would always be 6, so it is not returned"

    ivec3 imageCoords = ivec3(gl_GlobalInvocationID);
    ivec2 dims = ivec2(imageSize(Tex));

    vec3 dir = getCubeDirection(imageCoords, dims);
    vec3 n = vec3(getNoiseValue(dir));

    imageStore(Tex, imageCoords, vec4(n, 0));
}
