#include "matops/gemm.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdexcept>

#include "../gebp/gebp.hpp"
#include "../gebp/gebp_config.hpp"
#include "gemm_config.hpp"

namespace matops::detail {
namespace {

template <typename T>
void pack_a(index rows, index Kp, const T* source, index row_stride,
            index column_stride, T* packed) {
  index packed_index = 0;
  for (index i = 0; i < rows; i += gebp_config::MR) {
    const index rows_in_panel =
        std::min(gebp_config::MR, rows - i);
    for (index k = 0; k < Kp; ++k) {
      for (index r = 0; r < gebp_config::MR; ++r) {
        if (r < rows_in_panel) {
          packed[packed_index++] =
              source[(i + r) * row_stride + k * column_stride];
        } else {
          packed[packed_index++] = T(0);
        }
      }
    }
  }
}

template <typename T>
void pack_b(index Kp, index columns, const T* source, index row_stride,
            index column_stride, T* packed) {
  index packed_index = 0;
  for (index j = 0; j < columns; j += gebp_config::NR) {
    const index columns_in_panel =
        std::min(gebp_config::NR, columns - j);
    for (index k = 0; k < Kp; ++k) {
      for (index c = 0; c < gebp_config::NR; ++c) {
        if (c < columns_in_panel) {
          packed[packed_index++] =
              source[k * row_stride + (j + c) * column_stride];
        } else {
          packed[packed_index++] = T(0);
        }
      }
    }
  }
}

template <typename T>
void scale_c(T beta, matrix_view<T> C) {
  for (index i = 0; i < C.rows(); ++i) {
    for (index j = 0; j < C.columns(); ++j) {
      if (beta == T(0)) {
        C(i, j) = T(0);
      } else {
        C(i, j) *= beta;
      }
    }
  }
}

}  // namespace

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

  if (alpha == T(0) || K == 0) {
    scale_c(beta, C);
    return;
  }

  const index max_m = std::min(M, gemm_config::MC);
  const index max_n = std::min(N, gemm_config::NC);
  const index max_k = std::min(K, gemm_config::KC);
  const index packed_m =
      ((max_m + gebp_config::MR - 1) / gebp_config::MR) * gebp_config::MR;
  const index packed_n =
      ((max_n + gebp_config::NR - 1) / gebp_config::NR) * gebp_config::NR;
  auto A_packed = std::make_unique_for_overwrite<T[]>(
      static_cast<std::size_t>(packed_m * max_k));
  auto B_packed = std::make_unique_for_overwrite<T[]>(
      static_cast<std::size_t>(packed_n * max_k));

  for (index j = 0; j < N; j += gemm_config::NC) {
    const index j_len = std::min(gemm_config::NC, N - j);
    for (index p = 0; p < K; p += gemm_config::KC) {
      const index p_len = std::min(gemm_config::KC, K - p);

      // Pack each B panel once, then reuse it across all M tiles.
      pack_b(p_len, j_len, &B(p, j), B.row_stride(), B.column_stride(),
             B_packed.get());

      for (index i = 0; i < M; i += gemm_config::MC) {
        const index i_len = std::min(gemm_config::MC, M - i);
        matrix_view<T> C_tile(&C(i, j), i_len, j_len, C.row_stride(),
                              C.column_stride());

        if (p == 0) {
          scale_c(beta, C_tile);
        }

        pack_a(i_len, p_len, &A(i, p), A.row_stride(), A.column_stride(),
               A_packed.get());
        gebp_packed(alpha, A_packed.get(), B_packed.get(), i_len, j_len,
                    p_len, C_tile);
      }
    }
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
                          matrix_view<const float>, float, matrix_view<float>);
template void gemm<double>(double, matrix_view<const double>,
                           matrix_view<const double>, double,
                           matrix_view<double>);

}  // namespace matops
