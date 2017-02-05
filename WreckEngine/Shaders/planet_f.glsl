#version 450

out vec4 fragColor;

in vec3 fragNormal;
in vec3 tessCoord;

const vec3 LightPosition = vec3(4,0,0);
const vec3 DiffuseMaterial = vec3(0, 0.5, 0.5);
const vec3 AmbientMaterial = vec3(0.01,0.01,0);

void main()
{
    vec3 N = fragNormal;
    vec3 L = LightPosition;
    float df = abs(dot(N, L));
    vec3 color = AmbientMaterial + df * DiffuseMaterial;

    fragColor = vec4(fragNormal, 1.0);
}