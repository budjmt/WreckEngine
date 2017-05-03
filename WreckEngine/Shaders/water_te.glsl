#version 450

#include height.inc

layout(triangles, fractional_odd_spacing, ccw) in;

in vec3 tcPosition[];

out vec3 fragNormal;
out vec2 fragUV;
out vec3 tessCoord;

layout(location = 3) uniform mat4 iTworldMatrix;
layout(location = 4) uniform mat4 cameraMatrix;

patch in vec2 tcUV;
patch in float radius;
patch in vec3 teCamPos;

out vec3 position;

void main()
{
    vec3 p0 = gl_TessCoord.x * tcPosition[0];
    vec3 p1 = gl_TessCoord.y * tcPosition[1];
    vec3 p2 = gl_TessCoord.z * tcPosition[2];
    tessCoord = gl_TessCoord;

    fragUV = tcUV;

    fragNormal = normalize(p0 + p1 + p2);
    position = fragNormal * radius; // normalizing makes the c-sphere

    float dist = distance(position, teCamPos);
    float height = radius - 1.97;
    position += getHeight(fragNormal, max(height, 0), dist); // remove that clamp later?

    gl_Position = cameraMatrix * vec4(position, 1);
}