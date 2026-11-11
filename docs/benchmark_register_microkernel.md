# 4x4 Register Microkernel Benchmark

This experiment integrates a scalar 4x4 register-blocked microkernel into
GEBP, with the scalar path retained for edge tiles. These measurements compare
it with the scalar GEBP baseline recorded in
[`benchmark_baseline.md`](benchmark_baseline.md).

## Method and environment

The benchmark uses deterministic `double` inputs, checks each result against a
reference before timing, performs two warmup calls, then reports the median of
seven single-call samples. Both versions use AppleClang 21.0.0.21000334, C++20,
arm64 on Darwin 27.2.0, and CMake 4.1.2. The default build has an empty
`CMAKE_BUILD_TYPE` and no C++ optimization flags; Release uses `-O3 -DNDEBUG`.

Shapes are `M x N x K` for `A[M x K] * B[K x N]`. GFLOP/s is computed from
`2 * M * N * K / median_seconds`.

## Microkernel results

| Case | Shape | Default median (ms) | Default GFLOP/s | Release median (ms) | Release GFLOP/s |
|---|---:|---:|---:|---:|---:|
| Small | 16 x 16 x 16 | 0.004 | 1.820 | 0.001 | 8.192 |
| Square | 64 x 64 x 64 | 0.210 | 2.495 | 0.047 | 11.145 |
| Square | 128 x 128 x 128 | 1.316 | 3.188 | 0.380 | 11.045 |
| Square | 256 x 256 x 256 | 9.424 | 3.561 | 1.757 | 19.100 |
| Square | 512 x 512 x 512 | 77.409 | 3.468 | 11.031 | 24.334 |
| Tall | 128 x 32 x 64 | 0.150 | 3.486 | 0.021 | 24.916 |
| Wide | 32 x 128 x 64 | 0.151 | 3.468 | 0.021 | 25.267 |
| M edge | 65 x 5 x 17 | 0.009 | 1.211 | 0.001 | 12.050 |
| N edge | 5 x 513 x 17 | 0.066 | 1.316 | 0.006 | 13.593 |
| K edge | 5 x 7 x 257 | 0.023 | 0.784 | 0.004 | 4.594 |

## Release comparison with scalar baseline

| Case | Scalar baseline GFLOP/s | Register microkernel GFLOP/s |
|---|---:|---:|
| Small | 3.641 | 8.192 |
| Square 64 | 5.533 | 11.145 |
| Square 128 | 6.194 | 11.045 |
| Square 256 | 12.507 | 19.100 |
| Square 512 | 16.369 | 24.334 |
| Tall | 14.665 | 24.916 |
| Wide | 21.291 | 25.267 |
| M edge | 3.445 | 12.050 |
| N edge | 16.745 | 13.593 |
| K edge | 3.427 | 4.594 |

The Release microkernel improves throughput for the square cases: about 2x at
64x64x64 and about 1.5x at 256x256x256 and 512x512x512. The N-edge case is
slower than the scalar baseline in this run; small and edge cases are sensitive
to timing noise and edge-fallback overhead. All benchmark correctness checks
passed. These are single-machine results, not tuning targets.
