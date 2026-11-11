#include "matops/gemm.hpp"

#include <algorithm>
#include <stdexcept>

#include "../gebp/gebp.hpp"
#include "gemm_config.hpp"

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

  if (M == 0 || N == 0) {
    return;
  }

  for (index j = 0; j < N; j += gemm_config::NC) {
    const index j_len = std::min(gemm_config::NC, N - j);
    for (index i = 0; i < M; i += gemm_config::MC) {
      const index i_len = std::min(gemm_config::MC, M - i);
      matrix_view<T> C_tile(&C(i, j), i_len, j_len, C.row_stride(),
                            C.column_stride());

      for (index ti = 0; ti < i_len; ++ti) {
        for (index tj = 0; tj < j_len; ++tj) {
          if (beta == T(0)) {
            C_tile(ti, tj) = T(0);
          } else {
            C_tile(ti, tj) *= beta;
          }
        }
      }

      if (alpha == T(0) || K == 0) {
        continue;
      }

      for (index p = 0; p < K; p += gemm_config::KC) {
        const index p_len = std::min(gemm_config::KC, K - p);
        matrix_view<const T> A_panel(&A(i, p), i_len, p_len, A.row_stride(),
                                     A.column_stride());
        matrix_view<const T> B_panel(&B(p, j), p_len, j_len, B.row_stride(),
                                     B.column_stride());

        gebp(alpha, A_panel, B_panel, C_tile);
      }
    }
  }
}

} // namespace matops::detail

namespace matops {

template <typename T>
  requires(std::same_as<T, float> || std::same_as<T, double>)
void gemm(T alpha, matrix_view<const T> A, matrix_view<const T> B, T beta,
          matrix_view<T> C) {
  detail::gemm_impl(alpha, A, B, beta, C);
}

template void gemm<float>(float, matrix_view<const float>,
                          matrix_view<const float>, float, matrix_view<float>);
template void gemm<double>(double, matrix_view<const double>,
                           matrix_view<const double>, double,
                           matrix_view<double>);

} // namespace matops
