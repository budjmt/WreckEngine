#version 450

out vec4 fragColor;

in vec3 fragNormal;
in vec3 tessCoord;

layout (binding = 1) uniform samplerCube heightMap;

const vec3 LightDirection = normalize(vec3(-1,-1,0));
const vec3 DiffuseMaterial = vec3(0, 0.5, 0.5);
const vec3 AmbientMaterial = vec3(0.01,0.01,0);

vec3 getElevationColor(in float h) {
    if(h > 0.65) return vec3(1);
    if(h > 0.30) return vec3(0.45, 0.25, 0);
    if(h > 0.05) return vec3(0, 1, 0); 
                 return vec3(0, 0.2, 1) * ((h + 1) / 1.05);
}

float height(in vec3 coord) { return texture(heightMap, coord).r; }

void main()
{
    vec3 N = fragNormal;
    vec3 L = LightDirection;
    float df = abs(dot(N, L));
    //vec3 color = AmbientMaterial + df * DiffuseMaterial;
    
    vec3 elevationColor = getElevationColor(height(fragNormal));
    //vec3 color = AmbientMaterial + df * elevationColor;
    vec3 color = elevationColor;
    
    fragColor = vec4(color, 1.0);
}