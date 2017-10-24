#include "Color.h"

namespace Color {
    using byte = unsigned char;

    static inline byte packFloat(float value) {
        return static_cast<byte>(glm::round(glm::clamp(value, 0.0f, 1.0f) * 255.0f));
    }

    static inline float unpackFloat(byte value) {
        return value * inv255;
    }

    packed_type pack(const value_type& color) {
        byte r = packFloat(color.r),
             g = packFloat(color.g),
             b = packFloat(color.b),
             a = packFloat(color.a);
        return (r << 24) | (g << 16) | (b << 8) | a;
    }

    value_type unpack(packed_type color) {
        byte r = (color >> 24) & 255,
             g = (color >> 16) & 255,
             b = (color >>  8) & 255,
             a = (color >>  0) & 255;
        return { unpackFloat(r), unpackFloat(g), unpackFloat(b), unpackFloat(a) };
    }
}
