#version 450

layout(triangles, equal_spacing, cw) in;

in vec3 tcPosition[];
out vec3 teNormal;

uniform mat4 ViewProjection;
uniform mat4 iTworldMatrix;
uniform float Radius;

void main()
{
    vec3 p0 = gl_TessCoord.x * tcPosition[0];
    vec3 p1 = gl_TessCoord.y * tcPosition[1];
    vec3 p2 = gl_TessCoord.z * tcPosition[2];

    vec3 normalizedLocalPosition = normalize(p0 + p1 + p2);
    vec3 localPosition = normalizedLocalPosition * Radius;
    vec4 worldPosition = ViewProjection * vec4(localPosition, 1.0);

    teNormal = normalize((iTworldMatrix * vec4(normalizedLocalPosition, 0)).xyz);

    gl_Position = worldPosition;
}
