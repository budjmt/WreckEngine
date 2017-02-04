#include "MarchMath.h"

#include <intrin.h>
#include <iomanip>
#include <sstream>

int sign(int i) { return glm::sign(i); } 
int bitsign(int i) { constexpr int signbit = 1 << 31; return i & signbit; }
float signf(float f) { return glm::sign(f); }
//inline float maxf(float a, float b) { _mm_store_ss(&a, _mm_max_ss(_mm_set_ss(a), _mm_set_ss(b))); return a; }
//inline float minf(float a, float b) { _mm_store_ss(&a, _mm_min_ss(_mm_set_ss(a), _mm_set_ss(b))); return a; }
//inline float clampf(float val, float min, float max) { _mm_store_ss(&val, _mm_min_ss(_mm_max_ss(_mm_set_ss(val), _mm_set_ss(min)), _mm_set_ss(max))); return val; }
inline float maxf(float a, float b) { return glm::max(a, b); }
inline float minf(float a, float b) { return glm::min(a, b); }
inline float clampf(float val, float min, float max) { return maxf(min, minf(val, max)); }

std::string to_string(const vec2& v) { return std::to_string(v.x) + "," + std::to_string(v.y); }
std::string to_string(const vec2& v, size_t precision) { 
	std::stringstream stream;
	stream << std::fixed << std::setprecision(precision) << v.x << "," << v.y;
	return stream.str(); 
}

std::string to_string(const vec3& v) { return std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z); }
std::string to_string(const vec3& v, size_t precision) {
	std::stringstream stream;
	stream << std::fixed << std::setprecision(precision) << v.x << "," << v.y << "," << v.z;
	return stream.str();
}

std::string to_string(const vec4& v) { return std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + "," + std::to_string(v.w); }
std::string to_string(const vec4& v, size_t precision) {
	std::stringstream stream;
	stream << std::fixed << std::setprecision(precision) << v.x << "," << v.y << "," << v.z << "," << v.w;
	return stream.str();
}

std::string to_string(const quat& q) { return std::to_string(q.x) + "," + std::to_string(q.y) + "," + std::to_string(q.z) + "," + std::to_string(q.w); }
std::string to_string(const quat& q, size_t precision) {
	std::stringstream stream;
	stream << std::fixed << std::setprecision(precision) << q.x << "," << q.y << "," << q.z << "," << q.w;
	return stream.str();
}

//mat4
//based on the transform inverse shortcut where the mat is [M 0, v 1] (rows) and the inverse is [M^-1 0, -M^-1*v 1]
mat4 inv_tp_tf(const mat4& m) {
	//the reason we do it this way is to save operations
	vec4 c0 = m[0], c1 = m[1], c2 = m[2], c3 = m[3];
	float a = c0.x, b = c1.x, c = c2.x, x = c3.x
		, d = c0.y, e = c1.y, f = c2.y, y = c3.y
		, g = c0.z, h = c1.z, i = c2.z, z = c3.z;
	float ei_fh = e*i - f*h, fg_di = f*g - d*i, dh_eg = d*h - e*g;
	float det = a * ei_fh + b * fg_di + c * dh_eg, _det = 1.f / det;
	//this is already transposed
	auto tc0 = vec4(ei_fh * _det, (c*h - b*i) * _det, (b*f - c*e) * _det, 0);
	auto tc1 = vec4(fg_di * _det, (a*i - c*g) * _det, (c*d - a*f) * _det, 0);
	auto tc2 = vec4(dh_eg * _det, (b*g - a*h) * _det, (a*e - b*d) * _det, 0);
	auto tc3 = vec4(0,            0,                  0,                  1);
	tc0.w  = -(tc0.x * x + tc0.y * y + tc0.z * z);
	tc1.w  = -(tc1.x * x + tc1.y * y + tc1.z * z);
	tc2.w  = -(tc2.x * x + tc2.y * y + tc2.z * z);
	return mat4(tc0, tc1, tc2, tc3);
}

// [from] and [to] are unit vectors representing directions
// http://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d
mat4 rotateBetween(const vec3 from, const vec3 to) {
	const auto c = glm::dot(from, to);// cosine of angle between unit vectors
	if (c == -1.f) return glm::rotate(PI, vec3(1,0,0));// for the case of vectors facing the complete opposite direction
	
	const auto v = glm::cross(from, to);// normal of the plane shared by the two vectors
	const auto vx = mat3(vec3(0, v.z, -v.y), vec3(-v.z, 0, v.x), vec3(v.y, -v.x, 0));
	return mat4(mat3() + vx + vx * vx / (1.f + c));
}

//quat
#include <cassert>
void quat::updateAngles() { _theta = acosf(v0); assert(!NaN_CHECK(_theta)); sin_t_half = sinf(_theta); if (sin_t_half) { _axis = v / sin_t_half; } _axis /= glm::length(_axis); _theta *= 2; }
void quat::updateValues(vec3& a, float t) { t *= 0.5f; sin_t_half = sinf(t); v0 = cosf(t); v = a * sin_t_half; }

quat::quat(float _x, float _y, float _z, float _w) : v(_x, _y, _z), v0(_w) { updateAngles(); }
quat::quat(float _v0, vec3 v1) : v(v1), v0(_v0) { updateAngles(); }
quat::quat(vec3 a, float t) : _theta(t), _axis(a) { updateValues(a, t); }

float quat::theta() const { return _theta; } void quat::theta(float t) { _theta = t; updateValues(_axis, t); }
vec3  quat::axis()  const { return _axis;  } void quat::axis(vec3 a)   { _axis = a; v = a * sin_t_half; }

quat quat::operator+(const quat& other) { return quat(v0 + other.v0, v + other.v); } quat quat::operator-(const quat& other) { return quat(v0 - other.v0, v - other.v); }
quat quat::operator*(float f) { return quat(v0 * f, v * f); } quat quat::operator/(float f) { return quat(v0 / f, v / f); }
quat quat::operator*(const quat& other) const { float val = v0 * other.v0 - dot(v, other.v); return quat(clampf(val, -1.f, 1.f), other.v * v0 + v * other.v0 + glm::cross(v, other.v)); }

float quat::length(const quat& q) { return (float) vec4(q.v0, q.v).length(); }

quat quat::pow(const quat& q, float e) { return quat::rotation(q._theta * e, q._axis); }
quat quat::inverse(const quat& q) { return quat(q.v0, -q.v); }
quat quat::rotate(const quat& q, float theta, vec3 axis) { return q * quat(axis, theta); }

quat quat::slerp(const quat& a, const quat& b, float t) { return quat::pow(b * quat::inverse(a), t) * a; }
quat quat::rotation(float theta, vec3 axis) { return quat(axis, theta); }
quat quat::point(float x, float y, float z) { return quat(x, y, z, 0); }