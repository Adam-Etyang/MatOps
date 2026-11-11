#pragma once

#include "matops/types.hpp"

namespace matops::detail {

// Updates a full 4x4 output tile from a K-depth panel. The pointers refer to
// the logical top-left elements; signed strides are supported.
template <typename T>
inline void microkernel_4x4(index K_depth, T alpha, const T *A, index A_rs,
                            index A_cs, const T *B, index B_rs, index B_cs,
                            T *C, index C_rs, index C_cs) {
  T c00 = C[0 * C_rs + 0 * C_cs];
  T c01 = C[0 * C_rs + 1 * C_cs];
  T c02 = C[0 * C_rs + 2 * C_cs];
  T c03 = C[0 * C_rs + 3 * C_cs];
  T c10 = C[1 * C_rs + 0 * C_cs];
  T c11 = C[1 * C_rs + 1 * C_cs];
  T c12 = C[1 * C_rs + 2 * C_cs];
  T c13 = C[1 * C_rs + 3 * C_cs];
  T c20 = C[2 * C_rs + 0 * C_cs];
  T c21 = C[2 * C_rs + 1 * C_cs];
  T c22 = C[2 * C_rs + 2 * C_cs];
  T c23 = C[2 * C_rs + 3 * C_cs];
  T c30 = C[3 * C_rs + 0 * C_cs];
  T c31 = C[3 * C_rs + 1 * C_cs];
  T c32 = C[3 * C_rs + 2 * C_cs];
  T c33 = C[3 * C_rs + 3 * C_cs];

  for (index k = 0; k < K_depth; ++k) {
    const T a0 = alpha * A[0 * A_rs + k * A_cs];
    const T a1 = alpha * A[1 * A_rs + k * A_cs];
    const T a2 = alpha * A[2 * A_rs + k * A_cs];
    const T a3 = alpha * A[3 * A_rs + k * A_cs];
    const T b0 = B[k * B_rs + 0 * B_cs];
    const T b1 = B[k * B_rs + 1 * B_cs];
    const T b2 = B[k * B_rs + 2 * B_cs];
    const T b3 = B[k * B_rs + 3 * B_cs];

    c00 += a0 * b0;
    c01 += a0 * b1;
    c02 += a0 * b2;
    c03 += a0 * b3;
    c10 += a1 * b0;
    c11 += a1 * b1;
    c12 += a1 * b2;
    c13 += a1 * b3;
    c20 += a2 * b0;
    c21 += a2 * b1;
    c22 += a2 * b2;
    c23 += a2 * b3;
    c30 += a3 * b0;
    c31 += a3 * b1;
    c32 += a3 * b2;
    c33 += a3 * b3;
  }

  C[0 * C_rs + 0 * C_cs] = c00;
  C[0 * C_rs + 1 * C_cs] = c01;
  C[0 * C_rs + 2 * C_cs] = c02;
  C[0 * C_rs + 3 * C_cs] = c03;
  C[1 * C_rs + 0 * C_cs] = c10;
  C[1 * C_rs + 1 * C_cs] = c11;
  C[1 * C_rs + 2 * C_cs] = c12;
  C[1 * C_rs + 3 * C_cs] = c13;
  C[2 * C_rs + 0 * C_cs] = c20;
  C[2 * C_rs + 1 * C_cs] = c21;
  C[2 * C_rs + 2 * C_cs] = c22;
  C[2 * C_rs + 3 * C_cs] = c23;
  C[3 * C_rs + 0 * C_cs] = c30;
  C[3 * C_rs + 1 * C_cs] = c31;
  C[3 * C_rs + 2 * C_cs] = c32;
  C[3 * C_rs + 3 * C_cs] = c33;
}

} // namespace matops::detail
