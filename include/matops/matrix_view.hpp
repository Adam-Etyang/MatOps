#pragma once

#include "matops/types.hpp"

namespace matops {

// Non-owning, strided view. The caller is responsible for valid storage and
// indices.
template <typename T> class matrix_view {
public:
  constexpr matrix_view() noexcept = default;

  constexpr matrix_view(T *data, index rows, index columns, index row_stride,
                        index column_stride) noexcept
      : data_(data), rows_(rows), columns_(columns), row_stride_(row_stride),
        column_stride_(column_stride) {}

  [[nodiscard]] constexpr T *data() const noexcept { return data_; }
  [[nodiscard]] constexpr index rows() const noexcept { return rows_; }
  [[nodiscard]] constexpr index columns() const noexcept { return columns_; }
  [[nodiscard]] constexpr index row_stride() const noexcept {
    return row_stride_;
  }
  [[nodiscard]] constexpr index column_stride() const noexcept {
    return column_stride_;
  }

  constexpr T &operator()(index row, index column) const noexcept {
    return data_[row * row_stride_ + column * column_stride_];
  }

private:
  T *data_ = nullptr;
  index rows_ = 0;
  index columns_ = 0;
  index row_stride_ = 0;
  index column_stride_ = 0;
};

} // namespace matops
