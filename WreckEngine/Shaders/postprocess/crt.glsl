#version 400

in vec2 uv;

uniform vec2 resolution;
uniform float time;

uniform sampler2D pixels;

out vec4 fragColor;

// Emulated input resolution.
/*#if 0
// Fix resolution to set amount.
vec2 res = vec2(320.0, 160.0);
#else
// Optimize for resize.
vec2 res = resolution.xy / 6.0
#endif*/

// Hardness of scanline.
//  -8.0 = soft
// -16.0 = medium
//float hardScan = -8.0;

// Hardness of pixels in scanline.
// -2.0 = soft
// -4.0 = hard
float hardPix = -3.0;

// Amount of shadow mask.
//float maskDark = 0.5;
//float maskLight = 1.5;

//------------------------------------------------------------------------

// sRGB to Linear.
// Assuing using sRGB typed textures this should not be needed.
float ToLinear1(in float c) { return (c <= 0.04045) ? c / 12.92 : pow((c + 0.055) / 1.055, 2.4); }
vec3 ToLinear(in vec3 c) { return vec3(ToLinear1(c.r), ToLinear1(c.g), ToLinear1(c.b)); }

// Linear to sRGB.
// Assuing using sRGB typed textures this should not be needed.
float ToSrgb1(in float c) { return (c < 0.0031308 ? c * 12.92 : 1.055 * pow(c, 0.41666) - 0.055); }
vec3 ToSrgb(in vec3 c) { return vec3(ToSrgb1(c.r), ToSrgb1(c.g), ToSrgb1(c.b)); }

// Nearest emulated sample given floating point position and texel offset.
// Also zero's off screen.
vec3 Fetch(in vec2 pos, in vec2 off, in vec2 res) {
	pos = floor(pos * res + off) / res;
	vec2 a = abs(pos - vec2(0.5));
	if (max(a.x, a.y) > 0.5)
		return vec3(0.);
	else // shut up compiler
		return ToLinear(texture(pixels, pos.xy).rgb);
}

// Distance in emulated pixels to nearest texel.
vec2 Dist(in vec2 pos, in vec2 res) { 
	return vec2(0.5) - fract(pos * res); 
}

// 1D Gaussian.
float Gaus(in float pos, in float scale) { 
	return exp2(scale * pos * pos); 
}

// 3-tap Gaussian filter along horz line.
vec3 Horz3(in vec2 pos, in float off, in vec2 res) {
	vec3 b = Fetch(pos, vec2(-1.0, off), res);
	vec3 c = Fetch(pos, vec2( 0.0, off), res);
	vec3 d = Fetch(pos, vec2( 1.0, off), res);
	float dst = Dist(pos, res).x;
	// Convert distance to weight.
	float scale = hardPix;
	float wb = Gaus(dst - 1.0, scale);
	float wc = Gaus(dst + 0.0, scale);
	float wd = Gaus(dst + 1.0, scale);
	// Return filtered sample.
	return (b*wb + c*wc + d*wd) / (wb + wc + wd);
}

// 5-tap Gaussian filter along horz line.
vec3 Horz5(in vec2 pos, in float off, in vec2 res) {
	vec3 a = Fetch(pos, vec2(-2.0, off), res);
	vec3 b = Fetch(pos, vec2(-1.0, off), res);
	vec3 c = Fetch(pos, vec2( 0.0, off), res);
	vec3 d = Fetch(pos, vec2( 1.0, off), res);
	vec3 e = Fetch(pos, vec2( 2.0, off), res);
	float dst = Dist(pos, res).x;
	// Convert distance to weight.
	float scale = hardPix;
	float wa = Gaus(dst - 2.0, scale);
	float wb = Gaus(dst - 1.0, scale);
	float wc = Gaus(dst + 0.0, scale);
	float wd = Gaus(dst + 1.0, scale);
	float we = Gaus(dst + 2.0, scale);
	// Return filtered sample.
	return (a*wa + b*wb + c*wc + d*wd + e*we) / (wa + wb + wc + wd + we);
}

// Return scanline weight.
float Scan(in vec2 pos, in float off, in vec2 res, in float hardScan) {
	float dst = Dist(pos, res).y;
	return Gaus(dst + off, hardScan);
}

// Allow nearest three lines to effect pixel.
vec3 Tri(in vec2 pos, in vec2 res, in float hardScan) {
	vec3 a = Horz3(pos, -1.0, res);
	vec3 b = Horz5(pos,  0.0, res);
	vec3 c = Horz3(pos,  1.0, res);
	float wa = Scan (pos, -1.0, res, hardScan);
	float wb = Scan (pos,  0.0, res, hardScan);
	float wc = Scan (pos,  1.0, res, hardScan);
	return a*wa + b*wb + c*wc;
}

// Display warp.
// 0.0 = none
// 1.0/8.0 = extreme
vec2 warp = vec2(1.0 / 32.0, 1.0 / 24.0);
// Distortion of scanlines, and end of screen alpha.
vec2 Warp(in vec2 pos) {
	pos = (pos * 2.0) - 1.0;
	pos *= vec2(1.05) + (pos * pos).yx * warp;
	return (pos * 0.5) + 0.5;
}

// Shadow mask.
vec3 Mask(in vec2 pos, in float maskDark, in float maskLight) {
	pos.x += pos.y * 2.0; // change the const value to change the pixels a bit
	vec3 mask = vec3(maskDark);
	pos.x = fract(pos.x / 6.0);
	if (pos.x < 0.333)
		mask.r = maskLight;
	else if (pos.x < 0.666)
		mask.g = maskLight;
	else 
		mask.b = maskLight;
	return mask;
}

vec3 ScanMask(in float posY, in float maskDark, in float maskLight) {
	//return vec3(1.);
	//posY *= 3;
	float p = 0.75 * (modf(time * 0.3, 2.0) - 1) + 0.5;
	float r = 0.035;
	float d = abs(posY - p) - r;
	if (d > 0)
		return vec3(maskDark);
	else
		return vec3(maskLight * (1.5 - clamp(5 * exp2(d), 0, 1.5)));
}

//credit to https://www.shadertoy.com/view/XsjSzR#
void main() {
	
	vec2 res = resolution / 2.0;

	vec2 fragCoord = uv * resolution; // the coordinate in pixels
	
	// Hardness of scanline.
	//  -8.0 = soft
	// -16.0 = medium
	float hardScan = -8.0;

	// Amount of shadow mask.
	float maskDark = 0.5;
	float maskLight = 1.5;

	// Unmodified.
	vec2 pos = Warp(uv);
	//vec2 pos = uv;
	//return texture(pixels, pos) - texture(pixels, uv);
	//return vec4(pos,0,1);
	fragColor.rgb = Tri(pos, res, hardScan) 
				* Mask(fragCoord.xy, maskDark, maskLight) 
				+ ScanMask(uv.y, -maskDark * 0.05, maskLight * 0.05);
	
	fragColor.rgb = ToSrgb(fragColor.rgb);
	fragColor.a = texture(pixels, uv).a;
}
