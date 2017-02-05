#version 450

//layout(triangles, equal_spacing, ccw) in;
layout(triangles, fractional_odd_spacing, ccw) in;

in vec3 tcPosition[];
out vec3 tePosition;
out vec3 teNormal;
out vec3 teTessCoord;

layout (location = 4) uniform mat4 iTworldMatrix;
layout (location = 8) uniform mat4 cameraMatrix;

patch in float radius;

void main()
{
    vec3 p0 = gl_TessCoord.x * tcPosition[0];
    vec3 p1 = gl_TessCoord.y * tcPosition[1];
    vec3 p2 = gl_TessCoord.z * tcPosition[2];
    teTessCoord = gl_TessCoord;
    tePosition = normalize(p0 + p1 + p2) * radius; // normalizing makes the c-sphere

    teNormal = normalize((iTworldMatrix * vec4(tePosition, 0)).xyz);

    gl_Position = cameraMatrix * vec4(tePosition, 1);
}