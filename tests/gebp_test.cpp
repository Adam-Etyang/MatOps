#include <gtest/gtest.h>

#include <utility>

#include "gebp/gebp.hpp"
#include "matops/matrix.hpp"

TEST(GEBPTest, BasicAccumulation) {
  matops::matrix<double> A(2, 2);
  matops::matrix<double> B(2, 2);
  matops::matrix<double> C(2, 2);

  auto a = A.view();
  auto b = B.view();
  auto c = C.view();

  a(0, 0) = 1.0;
  a(0, 1) = 2.0;
  a(1, 0) = 2.0;
  a(1, 1) = 1.0;

  b(0, 0) = 1.0;
  b(0, 1) = 2.0;
  b(1, 0) = 2.0;
  b(1, 1) = 3.0;

  for (matops::index row = 0; row < c.rows(); ++row) {
    for (matops::index column = 0; column < c.columns(); ++column) {
      c(row, column) = 10.0;
    }
  }

  matops::detail::gebp(1.0, std::as_const(A).view(), std::as_const(B).view(), c);

  EXPECT_DOUBLE_EQ(c(0, 0), 15.0);
  EXPECT_DOUBLE_EQ(c(0, 1), 18.0);
  EXPECT_DOUBLE_EQ(c(1, 0), 14.0);
  EXPECT_DOUBLE_EQ(c(1, 1), 17.0);
}

TEST(GEBPTest, AlphaZeroLeavesCUnchanged) {
  matops::matrix<double> A(2, 2);
  matops::matrix<double> B(2, 2);
  matops::matrix<double> C(2, 2);

  auto c = C.view();
  c(0, 0) = 5.0;
  c(0, 1) = 6.0;
  c(1, 0) = 7.0;
  c(1, 1) = 8.0;

  matops::detail::gebp(0.0, std::as_const(A).view(), std::as_const(B).view(), c);

  EXPECT_DOUBLE_EQ(c(0, 0), 5.0);
  EXPECT_DOUBLE_EQ(c(0, 1), 6.0);
  EXPECT_DOUBLE_EQ(c(1, 0), 7.0);
  EXPECT_DOUBLE_EQ(c(1, 1), 8.0);
}

TEST(GEBPTest, RegisterTileAndEdgesMatchReference) {
  constexpr matops::index M = 5;
  constexpr matops::index N = 6;
  constexpr matops::index K = 3;
  constexpr double alpha = 0.75;

  matops::matrix<double> A(M, K);
  matops::matrix<double> B(K, N);
  matops::matrix<double> C(M, N);
  auto a = A.view();
  auto b = B.view();
  auto c = C.view();
  double expected[M][N];

  for (matops::index i = 0; i < M; ++i) {
    for (matops::index k = 0; k < K; ++k) {
      a(i, k) = 0.125 * (i + k + 1);
    }
  }
  for (matops::index k = 0; k < K; ++k) {
    for (matops::index j = 0; j < N; ++j) {
      b(k, j) = 0.0625 * (2 * k - j + 3);
    }
  }
  for (matops::index i = 0; i < M; ++i) {
    for (matops::index j = 0; j < N; ++j) {
      c(i, j) = 0.25 * (i - j);
      double product = 0.0;
      for (matops::index k = 0; k < K; ++k) {
        product += a(i, k) * b(k, j);
      }
      expected[i][j] = c(i, j) + alpha * product;
    }
  }

  matops::detail::gebp(alpha, std::as_const(A).view(), std::as_const(B).view(),
                       c);

  for (matops::index i = 0; i < M; ++i) {
    for (matops::index j = 0; j < N; ++j) {
      EXPECT_NEAR(c(i, j), expected[i][j], 1e-12);
    }
  }
}

TEST(GEBPTest, RegisterTileSupportsFloat) {
  float a_data[16]{};
  float b_data[16];
  float c_data[16]{};
  for (matops::index i = 0; i < 4; ++i) {
    a_data[i * 4 + i] = 1.0f;
    for (matops::index j = 0; j < 4; ++j) {
      b_data[i * 4 + j] = static_cast<float>(i * 4 + j + 1);
    }
  }

  matops::matrix_view<const float> A(a_data, 4, 4, 4, 1);
  matops::matrix_view<const float> B(b_data, 4, 4, 4, 1);
  matops::matrix_view<float> C(c_data, 4, 4, 4, 1);
  matops::detail::gebp(1.0f, A, B, C);

  for (matops::index i = 0; i < 4; ++i) {
    for (matops::index j = 0; j < 4; ++j) {
      EXPECT_FLOAT_EQ(C(i, j), b_data[i * 4 + j]);
    }
  }
}

TEST(GEBPTest, RegisterTileSupportsNonUnitStrides) {
  constexpr matops::index M = 4;
  constexpr matops::index N = 4;
  constexpr matops::index K = 3;
  constexpr double alpha = 0.5;

  matops::matrix<double> A_storage(M, 2 * K);
  matops::matrix<double> B_storage(K, 2 * N);
  matops::matrix<double> C_storage(M, 2 * N);
  auto a_storage = A_storage.view();
  auto b_storage = B_storage.view();
  auto c_storage = C_storage.view();
  double expected[M][N];

  for (matops::index i = 0; i < M; ++i) {
    for (matops::index k = 0; k < K; ++k) {
      a_storage(i, 2 * k) = 0.125 * (i + 2 * k + 1);
    }
  }
  for (matops::index k = 0; k < K; ++k) {
    for (matops::index j = 0; j < N; ++j) {
      b_storage(k, 2 * j) = 0.25 * (2 * k - j + 3);
    }
  }
  for (matops::index i = 0; i < M; ++i) {
    for (matops::index j = 0; j < N; ++j) {
      c_storage(i, 2 * j) = 0.5 * (i - j);
      double product = 0.0;
      for (matops::index k = 0; k < K; ++k) {
        product += a_storage(i, 2 * k) * b_storage(k, 2 * j);
      }
      expected[i][j] = c_storage(i, 2 * j) + alpha * product;
    }
  }

  matops::matrix_view<const double> A(A_storage.data(), M, K, 2 * K, 2);
  matops::matrix_view<const double> B(B_storage.data(), K, N, 2 * N, 2);
  matops::matrix_view<double> C(C_storage.data(), M, N, 2 * N, 2);
  matops::detail::gebp(alpha, A, B, C);

  for (matops::index i = 0; i < M; ++i) {
    for (matops::index j = 0; j < N; ++j) {
      EXPECT_NEAR(C(i, j), expected[i][j], 1e-12);
    }
  }
}
