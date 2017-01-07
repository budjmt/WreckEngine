#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <cmath>
#include <string>

template<typename T> constexpr T pi = T(3.14159265358979323846);
#define PI_D pi<double>
#define PI   pi<float>

//all NaN related comparisons evaluate to false, this could be implemented as x != x
//but that could be optimized out; std::isnan(x) is part of the standard library
#define NaN_CHECK(x) std::isnan(x)

//use for floating point error correction
inline bool epsCheck(float x)  { return x < FLT_EPSILON && x > -FLT_EPSILON; }
inline bool epsCheck(double x) { return x < DBL_EPSILON && x > -DBL_EPSILON; }

int sign(int i); 
int bitsign(int i);
float signf(float f);
inline float maxf(float a, float b);
inline float minf(float a, float b);
inline float clampf(float val, float min, float max);

typedef union { float f; uint32_t i; } bfloat;// allows for binary ops on a float

typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
class quat;

std::string to_string(const vec2&);
std::string to_string(const vec3&);
std::string to_string(const vec4&);
std::string to_string(const quat&);

mat4 inv_tp_tf(const mat4& m);
mat4 rotateBetween(const vec3 from, const vec3 to);

class quat {
public:
	quat() {};
	quat(float _x, float _y, float _z, float _w);
	quat(float v0, vec3 _v); quat(vec3 a, float t);
	
	~quat() = default;
	quat(const quat& other) = default;
	quat(quat&& other) = default;
	quat& operator=(quat other) { _v = other._v; v0 = other.v0; _theta = other._theta; _axis = other._axis; return *this; }
	
	vec3& v = _v;
	float &x = _v.x, &y = _v.y, &z = _v.z, &w = v0;
	float theta() const; void theta(float t);
	vec3 axis() const; void axis(vec3 a);

	quat operator+(const quat& other); quat operator-(const quat& other);
	quat operator*(float f); quat operator/(float f);
	quat operator*(const quat& other) const;

	static float length(const quat& q);

	static quat pow(const quat& q, float e);
	static quat inverse(const quat& q);
	static quat rotate(const quat& q, float theta, vec3 axis);

	static quat slerp(const quat& a, const quat& b, float t);
	static quat rotation(float theta, vec3 axis);
	static quat point(float x, float y, float z);

private:
	float v0 = 1;
	vec3 _v;
	float _theta = 0, sin_t_half = 0; vec3 _axis = vec3(0, 0, 1);

	inline void updateAngles();
	inline void updateValues(vec3& a, float t);
};