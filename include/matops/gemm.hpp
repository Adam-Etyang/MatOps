#pragma once

#include <concepts>

#include "matops/matrix_view.hpp"

namespace matops {

// Computes C = alpha * A * B + beta * C.
// The initial compiled implementation supports float and double.
template <typename T>
  requires(std::same_as<T, float> || std::same_as<T, double>)
void gemm(
    T alpha,
    matrix_view<const T> A,
    matrix_view<const T> B,
    T beta,
    matrix_view<T> C);

extern template void gemm<float>(
    float alpha,
    matrix_view<const float> A,
    matrix_view<const float> B,
    float beta,
    matrix_view<float> C);

extern template void gemm<double>(
    double alpha,
    matrix_view<const double> A,
    matrix_view<const double> B,
    double beta,
    matrix_view<double> C);

}  // namespace matops
