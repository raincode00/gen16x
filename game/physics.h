#pragma once

#include "2dmath.h"

struct BoxCollider {
    vec2 origin;
    vec2 size;
};

struct ConvexCollider {
    int num_edges;
    vec2 edges[16];
    vec2 origin;
};


extern "C" bool collision_box_box(const BoxCollider& box0, const BoxCollider& box1);
extern "C" int  closest_point_to_convex(const vec2& p, const ConvexCollider& conv, vec2* out_p);
extern "C" bool collision_box_convex(const BoxCollider& box, const ConvexCollider& conv);