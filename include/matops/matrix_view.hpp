#pragma once

#include "matops/types.hpp"

namespace matops {

// Non-owning, strided view. Storage and bounds policy are intentionally left
// for the implementation phase.
template <typename T>
class matrix_view {
public:
    constexpr matrix_view() noexcept = default;

    constexpr matrix_view(
        T* data,
        index rows,
        index columns,
        index row_stride,
        index column_stride) noexcept;

    [[nodiscard]] constexpr T* data() const noexcept;
    [[nodiscard]] constexpr index rows() const noexcept;
    [[nodiscard]] constexpr index columns() const noexcept;
    [[nodiscard]] constexpr index row_stride() const noexcept;
    [[nodiscard]] constexpr index column_stride() const noexcept;

    constexpr T& operator()(index row, index column) const noexcept;

private:
    T* data_ = nullptr;
    index rows_ = 0;
    index columns_ = 0;
    index row_stride_ = 0;
    index column_stride_ = 0;
};

}  // namespace matops
