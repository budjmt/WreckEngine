#version 450

layout (location = 0) out vec4 fragPosition;
layout (location = 1) out vec4 fragNormalized;
layout (location = 2) out vec4 diffColor;
layout (location = 3) out vec4 specColor;

in vec3 position;
in vec3 fragNormal;
in vec3 tessCoord;

layout (binding = 1) uniform samplerCube heightMap;
layout (binding = 2) uniform samplerCube normalMap;

vec3 getElevationColor(in float h) {
    if(h > 0.650) return vec3(1);
    if(h > 0.275) return vec3(0.45, 0.25, 0);
    if(h > 0.020) return vec3(0, 1, 0);
                  return vec3(0, 0.2, 1) * ((h + 1) / 1.05);
}

float height(in vec3 coord) {
    return texture(heightMap, coord).r;
}

void main()
{
    vec3 N = texture(normalMap, fragNormal).rgb;
    vec3 No = N;
    //N = vec3(0);

    vec3 elevationColor = getElevationColor(height(fragNormal));
    //vec3 color = fragNormal;
    //vec3 color = vec3(height(fragNormal));
    vec3 color = elevationColor;
    //vec3 color = N;
    //vec3 color = No;
    //vec3 color = vec3(1);

    fragPosition = vec4(position, 1);
    fragNormalized = vec4(N, 1);
    diffColor = vec4(color, 1);
    specColor = vec4(color / 4, 1);
}
