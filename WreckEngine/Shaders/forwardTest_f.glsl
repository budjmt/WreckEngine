#version 450

#include light/forward.inc

//uniform sampler2D tex;
uniform vec4 tint;

uniform vec3 camPos;

in vec4 worldPos;
in vec3 normal;
in vec2 uv;

layout (location = 0) out vec4 fragColor;

void main() {
	vec3 viewDir = normalize(camPos - worldPos.xyz);
	
    vec4 diffuse = vec4(0,0,0,1), specular = vec4(0,0,0,1);
	processLights(LightData(worldPos.xyz, viewDir, normal), diffuse, specular);
	
	fragColor = tint;
	fragColor = diffuse * fragColor + specular;
}