#include "matops/matrix.hpp"

namespace matops {

// TODO: Add non-template matrix implementation once the storage and layout
// policy has been decided.

} // namespace matops

template <typename T> index matrix<T>::rows() const noexcept { return rows_; }

template <typename T> index matrix<T>::columns() const noexcept {
  return columns_;
}

template <typename T>
matrix<T>::matrix(index rows, index columns)
    : rows_(rows), columns_(columns), storage_(rows * columns) {}

template <typename T> T *matrix<T>::data() noexcept { return storage_.data(); }

template <typename T> const T *matrix<T>::data() const noexcept {
  return storage_.data();
}
template <typename T> matrix_view<T> matrix<T>::view() noexcept {
  return matrix_view<T>(storage_.data(), rows_, columns_);
}

template <typename T> matrix_view<const T> matrix<T>::view() const noexcept {
  return matrix_view<const T>(storage_.data(), rows_, columns_);
}
