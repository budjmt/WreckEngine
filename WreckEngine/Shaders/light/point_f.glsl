#version 450

in mat4 camMat;
in Point {
    vec3 position;
	vec3 color;
	vec2 falloff;
} light;

uniform vec3 camPos;
uniform vec2 resolution;

// object render targets
uniform sampler2D gPosition;
uniform sampler2D gNormal;

// light accumulation targets
layout (location = 1) out vec4 diffuseColor;
layout (location = 2) out vec4 specularColor;

void main() {
	vec2 uv = gl_FragCoord.xy / resolution;

	// if the normal is black, the fragment wasn't rendered to
	// i.e. light can't reflect off of this fragment
	vec3 normal  = texture(gNormal, uv).rgb;
	if(normal == vec3(0)) discard;

    vec4 pos      = texture(gPosition, uv);
	vec3 worldPos = pos.rgb;
    vec3 fragPos  = (camMat * pos).rgb;
	
	vec3 toLight  = light.position - worldPos;
	float dist    = length(toLight);
	vec3 lightDir = normalize(toLight);
	
	vec3 viewDir  = normalize(-fragPos);
	vec3 hDir     = normalize(lightDir + viewDir);
	
	float a       = max(dist - light.falloff.x, 0.);
	float atten   = 1. -  min(a / (light.falloff.y - light.falloff.x), 1.); // plain old linear attenuation
	
	vec3 diffuse  = max(dot(normal, lightDir), 0.) * atten * light.color;
	
	float spec    = pow(max(dot(normal, hDir), 0.), 16.);
	vec3 specular = spec * atten * light.color;
	
	diffuseColor  = vec4(diffuse, 1);
	specularColor = vec4(specular, 1);
}