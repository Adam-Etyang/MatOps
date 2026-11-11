# GEMM Benchmark Baseline

These results record the scalar, blocked GEMM implementation before kernel
optimization. They provide a point of comparison for later changes; they are
not intended as tuned performance targets.

## Method

The benchmark uses deterministic `double` inputs, verifies each result against
a scalar reference before timing, performs two warmup calls, then reports the
median of seven single-call samples. GFLOP/s is computed from
`2 * M * N * K / median_seconds`. Run it with:

```sh
./build/matops_benchmarks
```

The `default` build used an empty `CMAKE_BUILD_TYPE` and no C++ optimization
flags. The `Release` build used `-O3 -DNDEBUG`. Both were compiled with
AppleClang 21.0.0.21000334, C++20, for arm64 on Darwin 27.2.0 using CMake
4.1.2. Results are from one run on this machine; timings will vary with system
load and hardware.

## Results

Shapes are shown as `M x N x K` for `A[M x K] * B[K x N]`.

| Case | Shape | Default median (ms) | Default GFLOP/s | Release median (ms) | Release GFLOP/s |
|---|---:|---:|---:|---:|---:|
| Small | 16 x 16 x 16 | 0.039 | 0.209 | 0.002 | 3.641 |
| Square | 64 x 64 x 64 | 0.792 | 0.662 | 0.095 | 5.533 |
| Square | 128 x 128 x 128 | 6.320 | 0.664 | 0.677 | 6.194 |
| Square | 256 x 256 x 256 | 48.846 | 0.687 | 2.683 | 12.507 |
| Square | 512 x 512 x 512 | 420.171 | 0.639 | 16.399 | 16.369 |
| Tall | 128 x 32 x 64 | 0.801 | 0.654 | 0.036 | 14.665 |
| Wide | 32 x 128 x 64 | 0.801 | 0.654 | 0.025 | 21.291 |
| M edge | 65 x 5 x 17 | 0.019 | 0.582 | 0.003 | 3.445 |
| N edge | 5 x 513 x 17 | 0.133 | 0.656 | 0.005 | 16.745 |
| K edge | 5 x 7 x 257 | 0.028 | 0.642 | 0.005 | 3.427 |

All benchmark correctness checks passed in both builds. The default build
throughput stays near 0.6–0.7 GFLOP/s for the larger shapes, while the Release
build improves as the matrices grow. This is the compiler-optimized baseline;
the GEMM/GEBP kernel itself is still scalar.
