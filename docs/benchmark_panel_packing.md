# Minimal Panel Packing Benchmark

This run measures GEMM after adding A/B panel packing around the scalar 4x4
register microkernel. B panels are packed once per N/K panel and reused across
M tiles. Results are compared with the register-only measurements in
[`benchmark_register_microkernel.md`](benchmark_register_microkernel.md).

## Method and environment

The benchmark uses deterministic `double` inputs, checks each result against a
reference before timing, performs two warmup calls, then reports the median of
seven single-call samples. Both builds use AppleClang 21.0.0.21000334, C++20,
arm64 on Darwin 27.2.0, and CMake 4.1.2. The default build has an empty
`CMAKE_BUILD_TYPE` and no C++ optimization flags; Release uses `-O3 -DNDEBUG`.

Shapes are `M x N x K` for `A[M x K] * B[K x N]`.

## Results with panel packing

| Case | Shape | Default median (ms) | Default GFLOP/s | Release median (ms) | Release GFLOP/s |
|---|---:|---:|---:|---:|---:|
| Small | 16 x 16 x 16 | 0.006 | 1.415 | 0.001 | 13.107 |
| Square | 64 x 64 x 64 | 0.238 | 2.206 | 0.023 | 22.713 |
| Square | 128 x 128 x 128 | 1.544 | 2.717 | 0.403 | 10.408 |
| Square | 256 x 256 x 256 | 9.935 | 3.377 | 1.799 | 18.654 |
| Square | 512 x 512 x 512 | 78.368 | 3.425 | 11.021 | 24.356 |
| Tall | 128 x 32 x 64 | 0.168 | 3.122 | 0.023 | 22.837 |
| Wide | 32 x 128 x 64 | 0.170 | 3.085 | 0.023 | 22.550 |
| M edge | 65 x 5 x 17 | 0.009 | 1.233 | 0.001 | 12.063 |
| N edge | 5 x 513 x 17 | 0.065 | 1.337 | 0.008 | 10.959 |
| K edge | 5 x 7 x 257 | 0.022 | 0.835 | 0.004 | 4.112 |

## Release comparison with register-only path

| Case | Register-only GFLOP/s | Packed GFLOP/s |
|---|---:|---:|
| Small | 8.192 | 13.107 |
| Square 64 | 11.145 | 22.713 |
| Square 128 | 11.045 | 10.408 |
| Square 256 | 19.100 | 18.654 |
| Square 512 | 24.334 | 24.356 |
| Tall | 24.916 | 22.837 |
| Wide | 25.267 | 22.550 |
| M edge | 12.050 | 12.063 |
| N edge | 13.593 | 10.959 |
| K edge | 4.594 | 4.112 |

The packing path shows mixed results in this run: the 64-square case improved,
the 512-square case was essentially unchanged, and several other cases were
slower. Small and edge cases are especially sensitive to timing noise. All
benchmark correctness checks passed; further measurements and tuning are
needed before treating packing as a performance improvement.
