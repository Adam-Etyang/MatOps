#pragma once

#include "matops/gebp.hpp"
#include "matops/matrix.hpp"
#include "matops/matrix_view.hpp"
#include "matrix_view.hpp"
#include <algorithm>

namespace matops {

// Computes C = alpha * A * B + beta * C.
// Dimension checks, aliasing rules, dispatch, and kernels are TODO.
template <typename T>
void gemm(T alpha, matrix_view<const T> A, matrix_view<const T> B, T beta,
          matrix_view<T> C) {

  const index M = A.rows();
  const index K = A.columns();
  const index N = B.columns();

  assert(B.rows() == K);
  assert(C.rows() == M);
  assert(C.columns() == N);

  for (index i = 0; i < M; i++) {
    for (index j = 0; j < N; j++) {
      if (beta == T(0)) {
        C(i, j) = T(0);
      } else {
        C(i, j) *= beta;
      }
    }
  }
  if (alpha == T(0) || K == 0 || N == 0 || M == 0) {
    return;
  }

  const index panelSize = 64;
  for (index k = 0; k < K; k += panelSize) {
    index Kp = std::min(panelSize, K - k);

    matrix_view<const T> A_panel(A.data() + k, M, Kp, A.row_stride(),
                                 A.column_stride());
    matrix_view<const T> B_panel(B.data() + (k * B.row_stride()), Kp, N,
                                 B.row_stride(), B.column_stride());

    matops::detail::gebp(alpha, A_panel, B_panel, C);
  }
}

} // namespace matops
