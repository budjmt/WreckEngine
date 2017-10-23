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
inline bool epsCheck(float x) { return x < FLT_EPSILON && x > -FLT_EPSILON; }
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

mat4 inv_tp_tf(const mat4& m);
mat4 rotateBetween(const vec3 from, const vec3 to);

class quat {
public:
    quat() : v(), v0(1) {};
    quat(float _x, float _y, float _z, float _w);
    quat(float v0, vec3 _v);
    explicit quat(vec4 _v);
    quat(vec3 a, float t);
    explicit quat(const mat4& rot);

    quat(const quat&) = default;
    quat& operator=(const quat&) = default;
    quat(quat&&) = default;
    quat& operator=(quat&&) = default;

    union {
        struct { vec4 vals; };
        struct { vec3 v; float v0; };
        struct { float x, y, z, w; };
    };

    float theta() const; void theta(float t);
    vec3 axis() const; void axis(vec3 a);

    quat& operator+=(const quat& other);
    quat& operator-=(const quat& other);
    quat& operator*=(float f);
    quat& operator/=(float f);
    quat& operator*=(const quat& other);

    static float length(const quat& q);

    static quat pow(const quat& q, float e);
    static quat inverse(const quat& q);
    static quat rotate(const quat& q, float theta, vec3 axis);

    static quat slerp(const quat& a, const quat& b, float t);
    static quat rotation(float theta, vec3 axis);
    static quat point(float x, float y, float z);

    static vec3 getEuler(const quat& q);
    static float getEulerX(const quat& q);
    static float getEulerY(const quat& q);
    static float getEulerZ(const quat& q);

private:
    float _theta = 0, sin_t_half = 0;
    vec3 _axis { 0, 0, 1 };

    inline void updateAngles();
    inline void updateValues(const vec3& a, float t);
};

quat operator+(const quat& a, const quat& b);
quat operator-(const quat& a, const quat& b);
quat operator*(const quat& q, float f);
inline quat operator*(float f, const quat& q) { return q * f; }
quat operator/(const quat& q, float f);
quat operator*(const quat& a, const quat& b);

template<typename T> inline std::string to_string(const T& obj) { return std::to_string(obj); }
template<> inline std::string to_string<vec2>(const vec2& v) { return std::to_string(v.x) + "," + std::to_string(v.y); };
std::string to_string(const vec2&, size_t precision);
template<> inline std::string to_string<vec3>(const vec3& v) { return std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z); };
std::string to_string(const vec3&, size_t precision);
template<> inline std::string to_string<vec4>(const vec4& v) { return std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + "," + std::to_string(v.w); };
std::string to_string(const vec4&, size_t precision);
template<> inline std::string to_string<quat>(const quat& q) { return std::to_string(q.x) + "," + std::to_string(q.y) + "," + std::to_string(q.z) + "," + std::to_string(q.w); };
std::string to_string(const quat&, size_t precision);

template<> inline std::string to_string<mat3>(const mat3& m) {
    auto col0 = m[0], col1 = m[1], col2 = m[2];
    return std::to_string(col0[0]) + "," + std::to_string(col1[0]) + "," + std::to_string(col2[0]) + "\n"
         + std::to_string(col0[1]) + "," + std::to_string(col1[1]) + "," + std::to_string(col2[1]) + "\n"
         + std::to_string(col0[2]) + "," + std::to_string(col1[2]) + "," + std::to_string(col2[2]);
}
template<> inline std::string to_string<mat4>(const mat4& m) {
    auto col0 = m[0], col1 = m[1], col2 = m[2], col3 = m[3];
    return std::to_string(col0[0]) + "," + std::to_string(col1[0]) + "," + std::to_string(col2[0]) + "," + std::to_string(col3[0]) + "\n"
         + std::to_string(col0[1]) + "," + std::to_string(col1[1]) + "," + std::to_string(col2[1]) + "," + std::to_string(col3[1]) + "\n"
         + std::to_string(col0[2]) + "," + std::to_string(col1[2]) + "," + std::to_string(col2[2]) + "," + std::to_string(col3[2]) + "\n"
         + std::to_string(col0[3]) + "," + std::to_string(col1[3]) + "," + std::to_string(col2[3]) + "," + std::to_string(col3[3]);
}