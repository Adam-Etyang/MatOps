#include "types.hpp"

template <typename T>
constexpr matrix_view<T>::matrix_view(T *data, index rows, index columns,
                                      index row_stride,
                                      index column_stride) noexcept
    : data_(data), rows_(rows), columns_(columns), row_stride_(row_stride),
      column_stride_(column_stride) {}

template <typename T> constexpr T *matrix_view<T>::data() const noexcept {
  return data_;
}

template <typename T> constexpr index matrix_view<T>::rows() const noexcept {
  return rows_;
}

template <typename T> constexpr index matrix_view<T>::columns() const noexcept {
  return columns_;
}

template <typename T>
constexpr index matrix_view<T>::row_stride() const noexcept {
  return row_stride_;
}

template <typename T>
constexpr index matrix_view<T>::column_stride_() const noexcept {
  return column_stride_;
}

template <typename T>
constexpr T &matrix_view<T>::operator()(index row,
                                        index column) const noexcept {
  retun data_[(row * row_stride_) + (column * column_stride_)];
}
