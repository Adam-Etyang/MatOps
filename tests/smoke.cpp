#include <gtest/gtest.h>

#include "matops/matrix.hpp"

TEST(MatrixTest, ViewUsesRowMajorStrides) {
    matops::matrix<int> values(2, 3);
    auto view = values.view();

    EXPECT_EQ(view.rows(), 2);
    EXPECT_EQ(view.columns(), 3);
    EXPECT_EQ(view.row_stride(), 3);
    EXPECT_EQ(view.column_stride(), 1);

    view(1, 2) = 42;
    EXPECT_EQ(values.data()[5], 42);
}
