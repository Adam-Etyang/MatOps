#pragma once

#include "matops/matrix_view.hpp"

namespace matops {

// Computes C = alpha * A * B + beta * C.
// Dimension checks, aliasing rules, dispatch, and kernels are TODO.
template <typename T>
void gemm(
    T alpha,
    matrix_view<const T> A,
    matrix_view<const T> B,
    T beta,
    matrix_view<T> C);

}  // namespace matops
