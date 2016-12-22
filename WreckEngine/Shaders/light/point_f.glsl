#version 400

in mat4 invCam;
in vec2 uv;
in Point {
    vec3 position;
	vec3 color;
	vec2 falloff;
} light;

uniform vec3 camPos;

// object render targets
uniform sampler2D gPosition;
uniform sampler2D gNormal;

// light accumulation targets
layout (location = 3) out vec4 diffuseColor;
layout (location = 4) out vec4 specularColor;

void main() {
    vec3 fragPos = texture(gPosition, uv).rgb;
	vec3 normal  = texture(gNormal, uv).rgb * 2. - 1.;
	
	vec3 toLight  = light.position - fragPos;
	float dist    = length(toLight);
	vec3 lightDir = normalize(toLight);
	
	vec3 viewDir  = normalize(camPos - fragPos);
	vec3 hDir     = normalize(lightDir + viewDir);
	
	float a       = max(dist - light.falloff.x, 0.);
	//float atten   = 1. -  a / (light.falloff.y - light.falloff.x); // plain old linear attenuation
	float atten = 1;
	
	vec3 diffuse  = max(dot(normal, lightDir), 0.) * atten * light.color;
	
	float spec    = pow(max(dot(normal, hDir), 0.), 16.);
	vec3 specular = spec * atten * light.color;
	
	//diffuseColor  = vec4(diffuse, 1.);
	//diffuseColor = vec4(fragPos, 1);
	//specularColor = vec4(specular, 1.);
	//specularColor = vec4(1);
}