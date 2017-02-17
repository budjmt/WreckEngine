#ifndef NOISE_COMMON
#define NOISE_COMMON

const float kInv289 = 1.0 / 289.0;

vec2 mod289(vec2 x)
{
    return x - floor(x * kInv289) * 289.0;
}

vec3 mod289(vec3 x)
{
    return x - floor(x * kInv289) * 289.0;
}

vec4 mod289(vec4 x)
{
    return x - floor(x * kInv289) * 289.0;
}

vec3 permute(vec3 x)
{
    return mod289((x * 34.0 + 1.0) * x);
}

vec4 permute(vec4 x)
{
    return mod289((x * 34.0 + 1.0) * x);
}

vec4 taylorInvSqrt(vec4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}

vec2 fade(vec2 t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

vec3 fade(vec3 t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

#endif
