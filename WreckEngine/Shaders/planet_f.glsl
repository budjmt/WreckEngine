#version 450

out vec4 fragColor;

in vec3 tePosition;
in vec3 teNormal;
in vec3 teTessCoord;

const vec3 LightPosition = vec3(4,0,0);
const vec3 DiffuseMaterial = vec3(0, 0.5, 0.5);
const vec3 AmbientMaterial = vec3(0.01,0.01,0);

void main()
{
    vec3 N = teNormal;
    vec3 L = LightPosition;
    float df = abs(dot(N, L));
    vec3 color = AmbientMaterial + df * DiffuseMaterial;

    fragColor = vec4(teNormal, 1.0);
}