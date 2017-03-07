#version 450

out vec4 fragColor;

in vec3 fragNormal;
in vec3 tessCoord;

in float height;

const vec3 LightPosition = vec3(4,0,0);
const vec3 DiffuseMaterial = vec3(0, 0.5, 0.5);
const vec3 AmbientMaterial = vec3(0.01,0.01,0);

vec3 getElevationColor(in float h) {
    if(h > 0.65) return vec3(1);
    if(h > 0.2)  return vec3(0.45, 0.25, 0);
                 return vec3(0, 1, 0); 
}

void main()
{
    vec3 N = fragNormal;
    vec3 L = LightPosition;
    float df = abs(dot(N, L));
    //vec3 color = AmbientMaterial + df * DiffuseMaterial;
    
    vec3 elevationColor = getElevationColor(height);
    vec3 color = AmbientMaterial + df * elevationColor;
    
    fragColor = vec4(color, 1.0);
}