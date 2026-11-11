#include "matops/gemm.hpp"

#include <algorithm>
#include <stdexcept>

#include "../gebp/gebp.hpp"

namespace matops::detail {

template <typename T>
void gemm_impl(T alpha, matrix_view<const T> A, matrix_view<const T> B, T beta,
               matrix_view<T> C) {
  if (A.rows() < 0 || A.columns() < 0 || B.rows() < 0 || B.columns() < 0 ||
      C.rows() < 0 || C.columns() < 0) {
    throw std::invalid_argument("GEMM dimensions must be non-negative");
  }

  const index M = A.rows();
  const index K = A.columns();
  const index N = B.columns();

  if (B.rows() != K || C.rows() != M || C.columns() != N) {
    throw std::invalid_argument("GEMM matrix dimensions are incompatible");
  }

  for (index i = 0; i < M; ++i) {
    for (index j = 0; j < N; ++j) {
      if (beta == T(0)) {
        C(i, j) = T(0);
      } else {
        C(i, j) *= beta;
      }
    }
  }

  if (alpha == T(0) || K == 0 || M == 0 || N == 0) {
    return;
  }

  constexpr index panel_size = 64;
  for (index k = 0; k < K; k += panel_size) {
    const index panel_k = std::min(panel_size, K - k);
    matrix_view<const T> A_panel(
        A.data() + k * A.column_stride(), M, panel_k, A.row_stride(),
        A.column_stride());
    matrix_view<const T> B_panel(
        B.data() + k * B.row_stride(), panel_k, N, B.row_stride(),
        B.column_stride());

    gebp(alpha, A_panel, B_panel, C);
  }
}

}  // namespace matops::detail

namespace matops {

template <typename T>
  requires(std::same_as<T, float> || std::same_as<T, double>)
void gemm(T alpha, matrix_view<const T> A, matrix_view<const T> B, T beta,
          matrix_view<T> C) {
  detail::gemm_impl(alpha, A, B, beta, C);
}

template void gemm<float>(float, matrix_view<const float>,
                          matrix_view<const float>, float,
                          matrix_view<float>);
template void gemm<double>(double, matrix_view<const double>,
                           matrix_view<const double>, double,
                           matrix_view<double>);

}  // namespace matops
