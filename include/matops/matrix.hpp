#pragma once

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

#include "matops/matrix_view.hpp"

namespace matops {

// Owning row-major matrix.
template <typename T> class matrix {
public:
  matrix() = default;
  matrix(index rows, index columns)
      : rows_(rows), columns_(columns),
        storage_(checked_element_count(rows, columns)) {}

  [[nodiscard]] index rows() const noexcept { return rows_; }
  [[nodiscard]] index columns() const noexcept { return columns_; }
  [[nodiscard]] T *data() noexcept { return storage_.data(); }
  [[nodiscard]] const T *data() const noexcept { return storage_.data(); }

  [[nodiscard]] matrix_view<T> view() noexcept {
    return matrix_view<T>(storage_.data(), rows_, columns_, columns_, 1);
  }
  [[nodiscard]] matrix_view<const T> view() const noexcept {
    return matrix_view<const T>(storage_.data(), rows_, columns_, columns_, 1);
  }

private:
  static std::size_t checked_element_count(index rows, index columns) {
    if (rows < 0 || columns < 0) {
      throw std::invalid_argument("matrix dimensions must be non-negative");
    }
    if (columns != 0 && rows > std::numeric_limits<index>::max() / columns) {
      throw std::length_error("matrix dimensions overflow");
    }
    return static_cast<std::size_t>(rows) * static_cast<std::size_t>(columns);
  }

  index rows_ = 0;
  index columns_ = 0;
  std::vector<T> storage_;
};

} // namespace matops
