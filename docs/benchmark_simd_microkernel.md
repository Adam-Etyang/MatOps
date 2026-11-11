# AArch64 SIMD Microkernel Benchmark

This run compares the packed scalar microkernel with the AArch64 NEON double
microkernel. SIMD is used for full 4x4 packed tiles when C has unit column
stride; the scalar microkernel and edge fallback remain available otherwise.

## Method and environment

The benchmark uses deterministic `double` inputs, checks each output against a
scalar reference before timing, performs two warmup calls, then reports the
median of seven single-call samples. The machine is arm64 on Darwin 27.2.0 with
AppleClang 21.0.0.21000334 and CMake 4.1.2. The default build has no C++
optimization flags; Release uses `-O3 -DNDEBUG`.

Shapes are `M x N x K` for `A[M x K] * B[K x N]`.

## SIMD results

| Case | Shape | Default median (ms) | Default GFLOP/s | Release median (ms) | Release GFLOP/s |
|---|---:|---:|---:|---:|---:|
| Small | 16 x 16 x 16 | 0.025 | 0.327 | 0.001 | 7.557 |
| Square | 64 x 64 x 64 | 1.122 | 0.467 | 0.037 | 14.122 |
| Square | 128 x 128 x 128 | 5.459 | 0.768 | 0.347 | 12.092 |
| Square | 256 x 256 x 256 | 36.671 | 0.915 | 1.412 | 23.768 |
| Square | 512 x 512 x 512 | 295.391 | 0.909 | 8.984 | 29.878 |
| Tall | 128 x 32 x 64 | 0.618 | 0.849 | 0.015 | 34.664 |
| Wide | 32 x 128 x 64 | 0.607 | 0.863 | 0.015 | 33.917 |
| M edge | 65 x 5 x 17 | 0.015 | 0.731 | 0.001 | 13.265 |
| N edge | 5 x 513 x 17 | 0.121 | 0.722 | 0.007 | 12.170 |
| K edge | 5 x 7 x 257 | 0.027 | 0.669 | 0.004 | 4.233 |

## Release comparison with packed scalar microkernel

| Case | Packed scalar GFLOP/s | SIMD GFLOP/s |
|---|---:|---:|
| Small | 13.107 | 7.557 |
| Square 64 | 22.713 | 14.122 |
| Square 128 | 10.408 | 12.092 |
| Square 256 | 18.654 | 23.768 |
| Square 512 | 24.356 | 29.878 |
| Tall | 22.837 | 34.664 |
| Wide | 22.550 | 33.917 |
| M edge | 12.063 | 13.265 |
| N edge | 10.959 | 12.170 |
| K edge | 4.112 | 4.233 |

All correctness checks passed. The SIMD path improves the larger square, tall,
and wide Release cases in this run, but regresses the small and 64-square cases.
The unoptimized build is substantially slower with this intrinsic path; use
Release results when judging SIMD throughput. These are single-run results and
need repeated measurements before tuning decisions.
