# MatOps

MatOps is a C++20 project built with CMake.

The project is structured around a GEMM implementation. Public interfaces are
in `include/matops`, and implementation layers are in `src`.

## Build

```sh
cmake -S . -B build
cmake --build build
```

GoogleTest-backed tests and GEMM benchmarks are enabled by default. CMake
uses an installed GoogleTest package when available, otherwise it fetches
GoogleTest v1.17.0. Disable either target with `-DMATOPS_BUILD_TESTS=OFF` or
`-DMATOPS_BUILD_BENCHMARKS=OFF`.

Run the tests with:

```sh
ctest --test-dir build --output-on-failure
```

Run the GEMM benchmarks with:

```sh
./build/matops_benchmarks
```

Run the executable with:

```sh
./build/matops
```

See `docs/architecture.md` for the implementation boundaries and open design
decisions. See `docs/benchmark_baseline.md`,
`docs/benchmark_register_microkernel.md`, `docs/benchmark_panel_packing.md`,
`docs/benchmark_simd_microkernel.md`, and `docs/gemm_block_tuning.md` for GEMM
benchmark and block-tuning results.

Run a Release block-size sweep with:

```sh
python3 scripts/tune_gemm_blocks.py
```
