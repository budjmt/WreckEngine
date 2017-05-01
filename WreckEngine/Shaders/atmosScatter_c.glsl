#version 450

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(rg32f, binding = 0) uniform image2D lookupTex;

layout (location = 0) uniform int numSamples = 50;
layout (location = 1) uniform float hAvgDensity = 0.25;
layout (location = 2) uniform vec2 atmosRadius; // x is inner, y is outer

const float pi = 3.14159265358979323846;

const float heightScale = 1 / (atmosRadius.y - atmosRadius.x);
float scaleHeight(in float h)  { return (h - atmosRadius.x) * heightScale; }
float atmosHeight(in vec2 loc) { return scaleHeight(length(loc)); }

float atmosDensity(in float h) { 
    return exp(-h / hAvgDensity);
}

void main() {
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    vec2 dims = vec2(imageSize(lookupTex));
    vec2 uv = vec2(coords) / dims;

    float h = uv.x, hr = h * (atmosRadius.y - atmosRadius.x) + atmosRadius.x;
    float theta = pi * uv.y;

    vec2 ray = vec2(sin(theta), cos(theta));
    float t = sqrt((atmosRadius.y * atmosRadius.y) - (hr * hr) * (ray.x * ray.x)) - hr * ray.y;
    float len = t / float(numSamples);
    vec2 inc = ray * len;

    vec2 data = vec2(atmosDensity(h), 0);

    vec2 start = vec2(0, hr), curr = start + inc;
    for(int i = 1; i < numSamples; ++i, curr += inc) {
        // multiply for the scaling factor in the integral
        data.y += len * atmosDensity(atmosHeight(curr));
    }
    data.y *= 4 * pi;
    imageStore(lookupTex, coords, vec4(data,0,0));
}