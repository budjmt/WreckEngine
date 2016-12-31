#version 450

in vec2 uv;
in mat4 camMat;
in Directional {
    vec3 direction;
	vec3 color;
} light;

uniform vec3 camPos;
uniform vec2 resolution;

// object render targets
uniform sampler2D gPosition;
uniform sampler2D gNormal;

// light accumulation targets
layout(location = 1) out vec4 diffuseColor;
layout(location = 2) out vec4 specularColor;

void main() {
	vec3 normal  = texture(gNormal, uv).rgb;
	if(normal == vec3(0)) discard;
	
	vec3 fragPos = (camMat * texture(gPosition, uv)).rgb;
	
	vec3 lightDir = -light.direction;
	
	vec3 viewDir  = normalize(-fragPos);
	vec3 hDir     = normalize(lightDir + viewDir);
	
	vec3 diffuse  = max(dot(normal, lightDir), 0.) * light.color;
	
	float spec    = pow(max(dot(normal, hDir), 0.), 16.);
	vec3 specular = spec * light.color;
	
	diffuseColor  = vec4(diffuse, 1);
	specularColor = vec4(specular, 1.);
}