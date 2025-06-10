/* SPDX-FileCopyrightText: 2025 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

#include "BLI_math_vector.hh"
#include "BLI_span.hh"
#include "BLI_vector.hh"
#include "DNA_curve_types.h"

namespace blender::bke {

void curve_simplify(const blender::Span<blender::float3> positions,
                    bool cyclic,
                    float epsilon,
                    blender::MutableSpan<bool> points_to_delete);

}
