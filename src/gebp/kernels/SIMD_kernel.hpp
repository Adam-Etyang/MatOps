#pragma once

#include "matops/types.hpp"

#if defined(__aarch64__) || defined(_M_ARM64)
#include "SIMD_kernel_double.hpp"

namespace matops::detail {

void simd_microkernel_4x4_float(index K_depth, float alpha, const float* A,
                                const float* B, float* C,
                                index C_row_stride);

}  // namespace matops::detail
#endif
