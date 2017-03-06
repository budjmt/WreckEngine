#version 450

//layout(triangles, equal_spacing, ccw) in;
layout(triangles, fractional_odd_spacing, ccw) in;

in vec3 tcPosition[];

out vec3 fragNormal;
out vec3 tessCoord;

layout (location = 4) uniform mat4 iTworldMatrix;
layout (location = 8) uniform mat4 cameraMatrix;

layout (location = 14, binding = 1) uniform samplerCube heightMap;
out float height;

patch in float radius;
patch in vec3 teCamPos;

void main()
{
    vec3 p0 = gl_TessCoord.x * tcPosition[0];
    vec3 p1 = gl_TessCoord.y * tcPosition[1];
    vec3 p2 = gl_TessCoord.z * tcPosition[2];
    tessCoord = gl_TessCoord;
    
    fragNormal = normalize(p0 + p1 + p2);
    vec3 pos = fragNormal * radius; // normalizing makes the c-sphere

    float dist = distance(pos, teCamPos);
    height = texture(heightMap, fragNormal).r;
    pos += fragNormal * (height / dist);
    
    gl_Position = cameraMatrix * vec4(pos, 1);
}