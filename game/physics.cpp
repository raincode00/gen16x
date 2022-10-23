#include "physics.h"


bool collision_box_box(const BoxCollider& box0, const BoxCollider& box1) {
  return !((box0.origin.x > box1.origin.x + box1.size.x)
    || (box0.origin.x + box0.size.x < box1.origin.x)
    || (box0.origin.y < box1.origin.y + box1.size.y)
    || (box0.origin.y + box0.size.y > box1.origin.y));

}

int closest_point_to_convex(const vec2& p, const ConvexCollider& conv, vec2* out_p) {
  float min_d = 0;
  int min_edge = 0;

  for (int i = 0; i < conv.num_edges; i++) {
    vec2 e0 = conv.edges[i] + conv.origin;
    int i1 = (i + 1) % conv.num_edges;
    vec2 e1 = conv.edges[i1] + conv.origin;

    vec2 p0 = closest_point_on_edge(p, e0, e1);

    float d = dot(p - p0, p - p0);

    if (d < min_d || i == 0) {
      min_d = d;
      if (out_p) {
        *out_p = p0;
      }
      min_edge = i;
    }
  }
  return min_edge;
}

bool collision_box_convex(const BoxCollider& box, const ConvexCollider& conv) {


  vec2 b[4] = {
      box.origin,
      box.origin + vec2(box.size.x, 0),
      box.origin + box.size,
      box.origin + vec2(0, box.size.y),
  };

  float sum = 0;
  for (int i = 0; i < conv.num_edges; i++) {
    vec2 e0 = conv.edges[i] + conv.origin;
    int i1 = (i + 1) % conv.num_edges;
    vec2 e1 = conv.edges[i1] + conv.origin;

    sum += (e1.x - e0.x) * (e1.y + e0.y);


    bool sep = true;
    for (int j = 0; j < 4; j++) {
      float d = signed_sqr_dist_to_edge(b[j], e0, e1);
      if (d < 0) {
        sep = false;
        break;
      }
    }
    if (sep) {
      return false;
    }
  }

  return true;
}