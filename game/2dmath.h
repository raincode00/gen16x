#pragma once
#include <math.h>

struct vec2 {
    float x;
    float y;
    vec2() = default;
    vec2(float xy) : x(xy), y(xy) {}
    vec2(float _x, float _y) : x(_x), y(_y) {}
    vec2(const vec2& v) = default;
    vec2(vec2&& v) = default;
    vec2& operator =(const vec2& other) = default;
    vec2& operator =(vec2&& other) = default;
};


inline vec2 operator +(const vec2& a, const vec2& b) {
    return vec2(a.x + b.x, a.y + b.y);
}

inline vec2& operator +=(vec2& a, const vec2& b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}


inline vec2 operator -(const vec2& a, const vec2& b) {
    return vec2(a.x - b.x, a.y - b.y);
}

inline vec2 operator -(const vec2& a) {
    return vec2(-a.x, -a.y);
}

inline vec2& operator -=(vec2& a, const vec2& b) {
    a.x -= b.x;
    a.y -= b.y;
    return a;
}


inline vec2 operator *(const vec2& a, const vec2& b) {
    return vec2(a.x*b.x, a.y*b.y);
}

inline vec2 operator *(float s, const vec2& a) {
    return vec2(s*a.x, s*a.y);
}
inline vec2 operator *(const vec2& a, float s) {
    return vec2(a.x*s, a.y*s);
}

inline vec2& operator *=(vec2& a, const vec2& b) {
    a.x *= b.x;
    a.y *= b.y;
    return a;
}

inline vec2& operator *=(vec2& a, float s) {
    a.x *= s;
    a.y *= s;
    return a;
}


inline vec2 operator /(const vec2& a, const vec2& b) {
    return vec2(a.x/b.x, a.y/b.y);
}

inline vec2 operator /(float s, const vec2& a) {
    return vec2(a.x/s, a.y/s);
}

inline vec2 operator /(const vec2& a, float s) {
    return vec2(a.x/s, a.y/s);
}


inline vec2& operator /=(vec2& a, const vec2& b) {
    a.x /= b.x;
    a.y /= b.y;
    return a;
}

inline vec2& operator /=(vec2& a, float s) {
    a.x /= s;
    a.y /= s;
    return a;
}


inline float dot(const vec2& a, const vec2& b) {
    return a.x*b.x + a.y*b.y;
}

inline float mag(const vec2& v) {
    return sqrtf(dot(v, v));
}

inline float distance(const vec2& a, const vec2& b) {
    return mag(a - b);
}


inline void normalize(vec2& v) {
    float m = mag(v);
    if (m != 0.0f) {
        v /= m;
    }
}

inline vec2 normalized(const vec2& v) {
    vec2 v0 = v;
    normalize(v0);
    return v0;
}

inline vec2 closest_point_on_edge(const vec2& p, const vec2& e0, const vec2& e1) { 
    vec2 c = p - e0;
    vec2 v = e1 - e0;

    float d = dot(v, v);
    float t = dot(v, c);

    if (d > 0.0f) {
        t /= d;
    }

    if (t < 0.0f) {
        t = 0.0f;
    } else if (t > 1.0f) {
        t = 1.0f;
    }
    return e0 + v*t;
}


inline float signed_sqr_dist_to_edge(const vec2& p, const vec2& e0, const vec2& e1) { 
    vec2 c = p - e0;
    vec2 v = e1 - e0;
    return dot(vec2(-v.y, v.x), c);
}

