#pragma once

#include "matops/types.hpp"

namespace matops::detail::gemm_config {

#ifndef MATOPS_GEMM_MC
#define MATOPS_GEMM_MC 64
#endif

#ifndef MATOPS_GEMM_NC
#define MATOPS_GEMM_NC 512
#endif

#ifndef MATOPS_GEMM_KC
#define MATOPS_GEMM_KC 256
#endif

inline constexpr index MC = MATOPS_GEMM_MC;
inline constexpr index NC = MATOPS_GEMM_NC;
inline constexpr index KC = MATOPS_GEMM_KC;

}  // namespace matops::detail::gemm_config
