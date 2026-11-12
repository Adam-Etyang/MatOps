#include "SIMD_kernel.hpp"

#if defined(__aarch64__) || defined(_M_ARM64)
#include <arm_neon.h>

namespace matops::detail {

void simd_microkernel_4x4_float(index K_depth, float alpha, const float* A,
                                const float* B, float* C,
                                index C_row_stride) {
  float32x4_t c0 = vld1q_f32(C + 0 * C_row_stride);
  float32x4_t c1 = vld1q_f32(C + 1 * C_row_stride);
  float32x4_t c2 = vld1q_f32(C + 2 * C_row_stride);
  float32x4_t c3 = vld1q_f32(C + 3 * C_row_stride);

  for (index k = 0; k < K_depth; ++k) {
    const float a0 = alpha * A[k * 4 + 0];
    const float a1 = alpha * A[k * 4 + 1];
    const float a2 = alpha * A[k * 4 + 2];
    const float a3 = alpha * A[k * 4 + 3];
    const float32x4_t b = vld1q_f32(B + k * 4);

    c0 = vfmaq_n_f32(c0, b, a0);
    c1 = vfmaq_n_f32(c1, b, a1);
    c2 = vfmaq_n_f32(c2, b, a2);
    c3 = vfmaq_n_f32(c3, b, a3);
  }

  vst1q_f32(C + 0 * C_row_stride, c0);
  vst1q_f32(C + 1 * C_row_stride, c1);
  vst1q_f32(C + 2 * C_row_stride, c2);
  vst1q_f32(C + 3 * C_row_stride, c3);
}

}  // namespace matops::detail
#endif
