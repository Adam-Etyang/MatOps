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
