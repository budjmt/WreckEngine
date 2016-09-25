#include "MarchMath.h"

#include <intrin.h>

int sign(int i) { return glm::sign(i); } 
int bitsign(int i) { const int signbit = 1 << 31; return i & signbit; }
float signf(float f) { return glm::sign(f); }
//inline float maxf(float a, float b) { _mm_store_ss(&a, _mm_max_ss(_mm_set_ss(a), _mm_set_ss(b))); return a; }
//inline float minf(float a, float b) { _mm_store_ss(&a, _mm_min_ss(_mm_set_ss(a), _mm_set_ss(b))); return a; }
//inline float clampf(float val, float min, float max) { _mm_store_ss(&val, _mm_min_ss(_mm_max_ss(_mm_set_ss(val), _mm_set_ss(min)), _mm_set_ss(max))); return val; }
inline float maxf(float a, float b) { return glm::max(a, b); }
inline float minf(float a, float b) { return glm::min(a, b); }
inline float clampf(float val, float min, float max) { return maxf(min, minf(val, max)); }

std::string to_string(const vec2& v) { return std::to_string(v.x) + "," + std::to_string(v.y); }
std::string to_string(const vec3& v) { return std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z); }
std::string to_string(const vec4& v) { return std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + "," + std::to_string(v.w); }
std::string to_string(const quat& q) { return std::to_string(q.x) + "," + std::to_string(q.y) + "," + std::to_string(q.z) + "," + std::to_string(q.w); }

//quat
#include <assert.h>
void quat::updateAngles() { _theta = acosf(v0); assert(!NaN_CHECK(_theta)); sin_t_half = sinf(_theta); if (sin_t_half) { _axis = _v / sin_t_half; } _axis /= glm::length(_axis); _theta *= 2; }
void quat::updateValues(vec3& a, float t) { t *= 0.5f; sin_t_half = sinf(t); v0 = cosf(t); _v = a * sin_t_half; }

quat::quat(float _x, float _y, float _z, float _w) : _v(vec3(_x, _y, _z)), v0(_w) { updateAngles(); }
quat::quat(float _v0, vec3 v1) : _v(v1), v0(_v0) { updateAngles(); }
quat::quat(vec3 a, float t) : _theta(t), _axis(a) { updateValues(a, t); }

float quat::theta() const { return _theta; } void quat::theta(float t) { _theta = t; updateValues(_axis, t); }
vec3  quat::axis()  const { return _axis;  } void quat::axis(vec3 a)   { _axis = a; _v = a * sin_t_half; }

quat quat::operator+(const quat& other) { return quat(v0 + other.v0, _v + other._v); } quat quat::operator-(const quat& other) { return quat(v0 - other.v0, _v - other._v); }
quat quat::operator*(float f) { return quat(v0 * f, _v * f); } quat quat::operator/(float f) { return quat(v0 / f, _v / f); }
quat quat::operator*(const quat& other) const { float val = v0 * other.v0 - glm::dot(_v, other._v); return quat(clampf(val, -1.f, 1.f), other._v * v0 + _v * other.v0 + glm::cross(_v, other._v)); }

float quat::length(const quat& q) { return sqrtf(q.v0 * q.v0 + q._v.x * q._v.x + q._v.y * q._v.y + q._v.z * q._v.z); }

quat quat::pow(const quat& q, float e) { return quat::rotation(q._theta * e, q._axis); }
quat quat::inverse(const quat& q) { return quat(q.v0, -q._v); }
quat quat::rotate(const quat& q, float theta, vec3 axis) { return q * quat(axis, theta); }

quat quat::slerp(const quat& a, const quat& b, float t) { return quat::pow(b * quat::inverse(a), t) * a; }
quat quat::rotation(float theta, vec3 axis) { return quat(axis, theta); }
quat quat::point(float x, float y, float z) { return quat(x, y, z, 0); }