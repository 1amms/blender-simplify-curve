/* SPDX-FileCopyrightText: 2025 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include "BKE_curve_simplify.hh"

#include "BLI_math_vector.hh"
#include "BLI_span.hh"
#include "BLI_vector.hh"
#include <cstdio>

namespace blender::bke {

/* Calculate the perpendicular distance of a point to a line. */
static float point_line_distance(const float3 &point,
                                 const float3 &line_start,
                                 const float3 &line_end)
{
  const float3 line = line_end - line_start;
  const float3 projected = line_start + line * math::dot(point - line_start, line) / math::dot(line, line);
  return math::distance(point, projected);
}

/* Recursive RDP simplification. */
static void rdp_recursive(const Span<float3> points,
                          const int start,
                          const int end,
                          float epsilon,
                          MutableSpan<bool> points_to_delete)
{
  if (end <= start + 1) {
    return;
  }

  float max_dist = 0.0f;
  int index = start;

  for (int i = start + 1; i < end; i++) {
    float dist = point_line_distance(points[i], points[start], points[end]);
    if (dist > max_dist) {
      max_dist = dist;
      index = i;
    }
  }

  if (max_dist > epsilon) {
    rdp_recursive(points, start, index, epsilon, points_to_delete);
    rdp_recursive(points, index, end, epsilon, points_to_delete);
  }
  else {
    for (int i = start + 1; i < end; i++) {
      points_to_delete[i] = true;
    }
  }
}

void curve_simplify(const Span<float3> points,
                    const bool is_cyclic,
                    const float epsilon,
                    MutableSpan<bool> points_to_delete)
{
  // ——— Debug log #1 ———
  printf("[curve_simplify] called with %zu points, cyclic=%d, epsilon=%.6f\n",
         points.size(),
         (int)is_cyclic,
         epsilon);

  if (points.size() < 3) {
    return;
  }

  points_to_delete.fill(false);

  if (is_cyclic) {
    /* For cyclic curves, handle loop by duplicating the first point at the end. */
    Vector<float3> extended_points;
    extended_points.extend(points);
    extended_points.append(points[0]);

    Vector<bool> extended_delete(points.size() + 1, false);
    rdp_recursive(
        extended_points.as_span(), 0, extended_points.size() - 1, epsilon, extended_delete);

    /* Transfer back to original array (excluding last duplicate point). */
    for (int i = 0; i < points.size(); i++) {
      points_to_delete[i] = extended_delete[i];
    }
  }
  else {
    rdp_recursive(points, 0, points.size() - 1, epsilon, points_to_delete);
  }

  {
    int deleted = 0;
    for (bool del : points_to_delete) {
      if (del)
        deleted++;
    }
    printf("[curve_simplify] marked %d of %zu points for deletion\n", deleted, points.size());
  }
}

}  // namespace blender::bke
