#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

#include "gemm/gemm_config.hpp"
#include "matops/gemm.hpp"
#include "matops/matrix.hpp"

namespace {

void expect_gemm_matches_reference(matops::index M, matops::index N,
                                   matops::index K) {
  constexpr double alpha = 1.25;
  constexpr double beta = -0.5;

  matops::matrix<double> A(M, K);
  matops::matrix<double> B(K, N);
  matops::matrix<double> C(M, N);
  auto a = A.view();
  auto b = B.view();
  auto c = C.view();

  for (matops::index i = 0; i < M; ++i) {
    for (matops::index k = 0; k < K; ++k) {
      a(i, k) = static_cast<double>((i * 3 + k * 2) % 11 - 5) * 0.125;
    }
  }
  for (matops::index k = 0; k < K; ++k) {
    for (matops::index j = 0; j < N; ++j) {
      b(k, j) = static_cast<double>((k * 5 + j * 3) % 13 - 6) * 0.0625;
    }
  }

  std::vector<double> expected(static_cast<std::size_t>(M * N));
  for (matops::index i = 0; i < M; ++i) {
    for (matops::index j = 0; j < N; ++j) {
      c(i, j) = static_cast<double>((i + 2 * j) % 7 - 3) * 0.25;
      double product = 0.0;
      for (matops::index k = 0; k < K; ++k) {
        product += a(i, k) * b(k, j);
      }
      expected[static_cast<std::size_t>(i * N + j)] =
          alpha * product + beta * c(i, j);
    }
  }

  matops::gemm(alpha, std::as_const(A).view(), std::as_const(B).view(), beta,
               c);

  for (matops::index i = 0; i < M; ++i) {
    for (matops::index j = 0; j < N; ++j) {
      SCOPED_TRACE(testing::Message() << "M=" << M << " N=" << N
                                      << " K=" << K << " i=" << i
                                      << " j=" << j);
      EXPECT_NEAR(c(i, j), expected[static_cast<std::size_t>(i * N + j)],
                  1e-10);
    }
  }
}

}  // namespace

TEST(GEMMTest, HandlesStridedAAcrossMultipleKPanels) {
  constexpr matops::index M = 2;
  constexpr matops::index K = matops::detail::gemm_config::KC + 1;
  constexpr matops::index N = 3;
  constexpr double alpha = 0.5;
  constexpr double beta = 1.5;

  matops::matrix<double> A_storage(M, 2 * K);
  matops::matrix<double> B(K, N);
  matops::matrix<double> C(M, N);

  auto a_storage_view = A_storage.view();
  auto b_view = B.view();
  auto c_view = C.view();

  for (matops::index i = 0; i < M; ++i) {
    for (matops::index k = 0; k < K; ++k) {
      a_storage_view(i, 2 * k) = 0.02 * (i + k % 7 + 1);
    }
  }
  for (matops::index k = 0; k < K; ++k) {
    for (matops::index j = 0; j < N; ++j) {
      b_view(k, j) = 0.01 * (k % 5 + j + 1);
    }
  }

  double expected[M][N];
  for (matops::index i = 0; i < M; ++i) {
    for (matops::index j = 0; j < N; ++j) {
      c_view(i, j) = 1.0 + i + j;
      double product = 0.0;
      for (matops::index k = 0; k < K; ++k) {
        product += a_storage_view(i, 2 * k) * b_view(k, j);
      }
      expected[i][j] = alpha * product + beta * c_view(i, j);
    }
  }

  matops::matrix_view<const double> A(
      A_storage.data(), M, K, 2 * K, 2);
  matops::gemm(alpha, A, std::as_const(B).view(), beta, c_view);

  for (matops::index i = 0; i < M; ++i) {
    for (matops::index j = 0; j < N; ++j) {
      EXPECT_NEAR(c_view(i, j), expected[i][j], 1e-12);
    }
  }
}

TEST(GEMMTest, ComputesProductAndAppliesAlphaAndBeta) {
  const float a_data[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
  const float b_data[] = {7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f};
  float c_data[] = {1.0f, 2.0f, 3.0f, 4.0f};

  matops::matrix_view<const float> A(a_data, 2, 3, 3, 1);
  matops::matrix_view<const float> B(b_data, 3, 2, 2, 1);
  matops::matrix_view<float> C(c_data, 2, 2, 2, 1);

  matops::gemm(2.0f, A, B, 0.5f, C);

  EXPECT_FLOAT_EQ(C(0, 0), 116.5f);
  EXPECT_FLOAT_EQ(C(0, 1), 129.0f);
  EXPECT_FLOAT_EQ(C(1, 0), 279.5f);
  EXPECT_FLOAT_EQ(C(1, 1), 310.0f);
}

TEST(GEMMTest, BetaZeroDoesNotReadOldC) {
  const double a_data[] = {2.0};
  const double b_data[] = {3.0};
  double c_data[] = {std::numeric_limits<double>::quiet_NaN()};

  matops::matrix_view<const double> A(a_data, 1, 1, 1, 1);
  matops::matrix_view<const double> B(b_data, 1, 1, 1, 1);
  matops::matrix_view<double> C(c_data, 1, 1, 1, 1);

  matops::gemm(1.0, A, B, 0.0, C);

  EXPECT_DOUBLE_EQ(C(0, 0), 6.0);
}

TEST(GEMMTest, AlphaZeroOnlyScalesC) {
  const double nan = std::numeric_limits<double>::quiet_NaN();
  const double a_data[] = {nan};
  const double b_data[] = {nan};
  double c_data[] = {4.0};

  matops::matrix_view<const double> A(a_data, 1, 1, 1, 1);
  matops::matrix_view<const double> B(b_data, 1, 1, 1, 1);
  matops::matrix_view<double> C(c_data, 1, 1, 1, 1);

  matops::gemm(0.0, A, B, 2.0, C);

  EXPECT_DOUBLE_EQ(C(0, 0), 8.0);
}

TEST(GEMMTest, ZeroKScalesCByBeta) {
  double c_data[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
  matops::matrix_view<const double> A(nullptr, 2, 0, 0, 1);
  matops::matrix_view<const double> B(nullptr, 0, 3, 3, 1);
  matops::matrix_view<double> C(c_data, 2, 3, 3, 1);

  matops::gemm(3.0, A, B, 2.0, C);

  const double expected[] = {2.0, 4.0, 6.0, 8.0, 10.0, 12.0};
  for (matops::index i = 0; i < 6; ++i) {
    EXPECT_DOUBLE_EQ(c_data[i], expected[i]);
  }
}

TEST(GEMMTest, RejectsIncompatibleDimensions) {
  const double a_data[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
  const double b_data[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
  double c_data[] = {0.0, 0.0, 0.0, 0.0};

  matops::matrix_view<const double> A(a_data, 2, 3, 3, 1);
  matops::matrix_view<const double> B(b_data, 4, 2, 2, 1);
  matops::matrix_view<double> C(c_data, 2, 2, 2, 1);

  EXPECT_THROW(matops::gemm(1.0, A, B, 0.0, C), std::invalid_argument);
}

TEST(GEMMTest, HandlesPartialMTile) {
  expect_gemm_matches_reference(matops::detail::gemm_config::MC + 1, 5, 7);
}

TEST(GEMMTest, HandlesPartialNTile) {
  expect_gemm_matches_reference(5, matops::detail::gemm_config::NC + 1, 7);
}

TEST(GEMMTest, HandlesPartialKPanel) {
  expect_gemm_matches_reference(5, 7, matops::detail::gemm_config::KC + 1);
}
