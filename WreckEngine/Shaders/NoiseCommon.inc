#ifndef NOISE_COMMON
#define NOISE_COMMON

const float kInv7 = 1.0 / 7.0;
const float kInv289 = 1.0 / 289.0;

vec2 mod289(in vec2 x)
{
    return x - floor(x * kInv289) * 289.0;
}

vec3 mod289(in vec3 x)
{
    return x - floor(x * kInv289) * 289.0;
}

vec4 mod289(in vec4 x)
{
    return x - floor(x * kInv289) * 289.0;
}

vec3 permute(in vec3 x)
{
    return mod289((x * 34.0 + 1.0) * x);
}

vec4 permute(in vec4 x)
{
    return mod289((x * 34.0 + 1.0) * x);
}

vec3 seededPermute(in vec3 x, in int seed)
{
    float mult = float((seed % 17) + 17);
    float shft = seed / 0.3141592654;
    return mod289((x * mult + fract(shft)) * x);
}

vec4 seededPermute(in vec4 x, in int seed)
{
    float mult = float((seed % 17) + 17);
    float shft = float((seed % 4096) * 0.3141592654);
    return mod289((x * mult + fract(shft)) * x);
}

vec4 taylorInvSqrt(in vec4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}

vec2 fade(in vec2 t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

vec3 fade(in vec3 t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

#endif
