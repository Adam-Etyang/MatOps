#pragma once

#include <cassert>
#include <type_traits>

#include "gebp_config.hpp"
#include "kernels/SIMD_kernel.hpp"
#include "kernels/microkernel_4x4.hpp"
#include "matops/matrix_view.hpp"

namespace matops::detail {

template <typename T>
void gebp_packed(T alpha, const T* A_packed, const T* B_packed, index M,
                 index N, index Kp, matrix_view<T> C_tile) {
  assert(C_tile.rows() == M);
  assert(C_tile.columns() == N);

  if (alpha == T(0) || M == 0 || N == 0 || Kp == 0) {
    return;
  }

  for (index i = 0; i < M; i += gebp_config::MR) {
    const index rows = M - i < gebp_config::MR ? M - i : gebp_config::MR;
    const T* A_micro =
        A_packed + (i / gebp_config::MR) * Kp * gebp_config::MR;

    for (index j = 0; j < N; j += gebp_config::NR) {
      const index columns = N - j < gebp_config::NR ? N - j : gebp_config::NR;
      const T* B_micro =
          B_packed + (j / gebp_config::NR) * Kp * gebp_config::NR;

      if (rows == gebp_config::MR && columns == gebp_config::NR) {
#if defined(__aarch64__) || defined(_M_ARM64)
        if constexpr (std::is_same_v<T, double>) {
          if (C_tile.column_stride() == 1) {
            simd_microkernel_4x4_double(
                Kp, alpha, A_micro, B_micro, &C_tile(i, j),
                C_tile.row_stride());
            continue;
          }
        } else if constexpr (std::is_same_v<T, float>) {
          if (C_tile.column_stride() == 1) {
            simd_microkernel_4x4_float(
                Kp, alpha, A_micro, B_micro, &C_tile(i, j),
                C_tile.row_stride());
            continue;
          }
        }
#endif
        microkernel_4x4(Kp, alpha, A_micro, 1, gebp_config::MR, B_micro,
                        gebp_config::NR, 1, &C_tile(i, j),
                        C_tile.row_stride(), C_tile.column_stride());
        continue;
      }

      for (index ti = 0; ti < rows; ++ti) {
        for (index tj = 0; tj < columns; ++tj) {
          for (index k = 0; k < Kp; ++k) {
            const T a = A_micro[k * gebp_config::MR + ti];
            const T b = B_micro[k * gebp_config::NR + tj];
            C_tile(i + ti, j + tj) += (alpha * a) * b;
          }
        }
      }
    }
  }
}

template <typename T>
void gebp(T alpha, matrix_view<const T> A_panel, matrix_view<const T> B_panel,
          matrix_view<T> C_tile) {
  assert(A_panel.columns() == B_panel.rows());
  assert(C_tile.rows() == A_panel.rows());
  assert(C_tile.columns() == B_panel.columns());

  if (alpha == T(0)) {
    return;
  }

  const index M = A_panel.rows();
  const index Kp = A_panel.columns();
  const index N = B_panel.columns();

  if (M == 0 || Kp == 0 || N == 0) {
    return;
  }

  for (index i = 0; i < M; i += gebp_config::MR) {
    const index rows = M - i < gebp_config::MR ? M - i : gebp_config::MR;
    for (index j = 0; j < N; j += gebp_config::NR) {
      const index columns = N - j < gebp_config::NR ? N - j : gebp_config::NR;
      if (rows == gebp_config::MR && columns == gebp_config::NR) {
        microkernel_4x4(Kp, alpha, &A_panel(i, 0), A_panel.row_stride(),
                        A_panel.column_stride(), &B_panel(0, j),
                        B_panel.row_stride(), B_panel.column_stride(),
                        &C_tile(i, j), C_tile.row_stride(),
                        C_tile.column_stride());
        continue;
      }

      for (index ti = 0; ti < rows; ++ti) {
        for (index tj = 0; tj < columns; ++tj) {
          for (index k = 0; k < Kp; ++k) {
            C_tile(i + ti, j + tj) +=
                (alpha * A_panel(i + ti, k)) * B_panel(k, j + tj);
          }
        }
      }
    }
  }
}

} // namespace matops::detail
