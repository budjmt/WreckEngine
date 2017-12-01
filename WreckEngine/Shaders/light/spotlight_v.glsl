#version 450

layout (location = 0) in vec3 vertPos;
layout (location = 1) in mat4 world;
layout (location = 5) in mat4 lightViewProj;
// skip 4 for mat4
layout (location = 9)  in vec3 position;
layout (location = 10) in int  isOff;
layout (location = 11) in vec3 direction;
layout (location = 12) in uint tag;
layout (location = 13) in vec4 falloff; // xy = radial, zw = length
layout (location = 14) in vec3 color;
layout (location = 15) in int  castsShadow;

uniform mat4 camera;

out Spotlight {
    vec3 position;
    vec3 direction;
    vec3 color;
    vec2 falloffRad;
    vec2 falloffLen;
    bool castsShadow;
} light;

void main() {
	if(isOff == 0) {
		gl_Position = camera * world * vec4(vertPos, 1.);
		
		light.position    = position;
		light.direction   = direction;
		light.color       = color;
		light.falloffRad  = falloff.xy;
		light.falloffLen  = falloff.zw;
        light.castsShadow = castsShadow != 0;
	}
	else {
		gl_Position = vec4(vec2(-2.), vec2(1.)); // causes the vertex to be clipped
	}
}