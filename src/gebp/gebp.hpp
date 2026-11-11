#pragma once

#include <cassert>

#include "matops/matrix_view.hpp"

namespace matops::detail {

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

  for (index i = 0; i < M; i++) {
    for (index k = 0; k < Kp; k++) {
      T temp = alpha * A_panel(i, k);
      for (index j = 0; j < N; j++) {
        C_tile(i, j) += temp * B_panel(k, j);
      }
    }
  }
}

} // namespace matops::detail
