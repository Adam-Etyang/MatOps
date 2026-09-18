#pragma once

#include <vector>

#include "matops/matrix_view.hpp"

namespace matops {

// Owning matrix interface. Allocation, layout, and invariant enforcement are
// intentionally left unimplemented for the project owner.
template <typename T>
class matrix {
public:
    matrix() = default;
    matrix(index rows, index columns);

    [[nodiscard]] index rows() const noexcept;
    [[nodiscard]] index columns() const noexcept;
    [[nodiscard]] T* data() noexcept;
    [[nodiscard]] const T* data() const noexcept;

    [[nodiscard]] matrix_view<T> view() noexcept;
    [[nodiscard]] matrix_view<const T> view() const noexcept;

private:
    index rows_ = 0;
    index columns_ = 0;
    std::vector<T> storage_;
};

}  // namespace matops
