#pragma once

#include "matops/types.hpp"

#if defined(__aarch64__) || defined(_M_ARM64)
#include <arm_neon.h>

namespace matops::detail {

// Accumulates one packed 4x4 double-precision tile. A is laid out as
// [K][MR], B as [K][NR], and C must have unit column stride.
inline void simd_microkernel_4x4_double(index K_depth, double alpha,
                                       const double* A, const double* B,
                                       double* C, index C_row_stride) {
  float64x2_t c0_lo = vld1q_f64(C + 0 * C_row_stride);
  float64x2_t c0_hi = vld1q_f64(C + 0 * C_row_stride + 2);
  float64x2_t c1_lo = vld1q_f64(C + 1 * C_row_stride);
  float64x2_t c1_hi = vld1q_f64(C + 1 * C_row_stride + 2);
  float64x2_t c2_lo = vld1q_f64(C + 2 * C_row_stride);
  float64x2_t c2_hi = vld1q_f64(C + 2 * C_row_stride + 2);
  float64x2_t c3_lo = vld1q_f64(C + 3 * C_row_stride);
  float64x2_t c3_hi = vld1q_f64(C + 3 * C_row_stride + 2);

  for (index k = 0; k < K_depth; ++k) {
    const double a0 = alpha * A[k * 4 + 0];
    const double a1 = alpha * A[k * 4 + 1];
    const double a2 = alpha * A[k * 4 + 2];
    const double a3 = alpha * A[k * 4 + 3];
    const float64x2_t b_lo = vld1q_f64(B + k * 4);
    const float64x2_t b_hi = vld1q_f64(B + k * 4 + 2);

    c0_lo = vfmaq_n_f64(c0_lo, b_lo, a0);
    c0_hi = vfmaq_n_f64(c0_hi, b_hi, a0);
    c1_lo = vfmaq_n_f64(c1_lo, b_lo, a1);
    c1_hi = vfmaq_n_f64(c1_hi, b_hi, a1);
    c2_lo = vfmaq_n_f64(c2_lo, b_lo, a2);
    c2_hi = vfmaq_n_f64(c2_hi, b_hi, a2);
    c3_lo = vfmaq_n_f64(c3_lo, b_lo, a3);
    c3_hi = vfmaq_n_f64(c3_hi, b_hi, a3);
  }

  vst1q_f64(C + 0 * C_row_stride, c0_lo);
  vst1q_f64(C + 0 * C_row_stride + 2, c0_hi);
  vst1q_f64(C + 1 * C_row_stride, c1_lo);
  vst1q_f64(C + 1 * C_row_stride + 2, c1_hi);
  vst1q_f64(C + 2 * C_row_stride, c2_lo);
  vst1q_f64(C + 2 * C_row_stride + 2, c2_hi);
  vst1q_f64(C + 3 * C_row_stride, c3_lo);
  vst1q_f64(C + 3 * C_row_stride + 2, c3_hi);
}

}  // namespace matops::detail
#endif
