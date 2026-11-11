#pragma once

#include "matops/types.hpp"

namespace matops::detail::gemm_config {

inline constexpr index MC = 64;
inline constexpr index NC = 512;
inline constexpr index KC = 256;

}  // namespace matops::detail::gemm_config
