# GEMM Block-Size Tuning

The Release sweep script tests `MC`, `NC`, and `KC` combinations using the
correctness-checked GEMM benchmark:

```sh
python3 scripts/tune_gemm_blocks.py
```

The default sweep tests `MC={32,64,128}`, `NC={256,512,1024}`, and
`KC={128,256,512}`. It scores each configuration with the geometric mean of
GFLOP/s for `square-64`, `square-128`, `square-256`, `square-512`, `tall`, and
`wide`. The three leading candidates and the current default are confirmed with
five additional benchmark runs each. Each benchmark case uses two warmups and
the median of seven samples, with five GEMM calls per sample. Detailed
measurements are written to `build/gemm_block_tuning.csv`.

## Initial result

Measured on AppleClang 21.0.0.21000334, arm64, Darwin 27.2.0, using Release
(`-O3 -DNDEBUG`) on 2026-11-12:

| MC | NC | KC | Confirmed geometric mean (GFLOP/s) |
|---:|---:|---:|---:|
| 64 | 512 | 256 | 30.777 |
| 128 | 256 | 256 | 30.629 |
| 64 | 256 | 128 | 30.579 |

The current defaults (`64, 512, 256`) narrowly led this sweep, so they remain
the selected settings. The top scores are close; treat the difference as
inconclusive until rerun under steadier system load or against a broader set of
workloads.
