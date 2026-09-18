#pragma once

#include <cstddef>

namespace matops {

using index = std::ptrdiff_t;

enum class layout {
    row_major,
    column_major,
};

struct shape {
    index rows = 0;
    index columns = 0;
};

}  // namespace matops
