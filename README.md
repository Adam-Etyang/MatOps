# MatOps

MatOps is a C++20 project built with CMake.

The project is structured around a GEMM implementation. The public interfaces
are in `include/matops`, implementation layers are in `src`, and the current
algorithm-specific files are intentionally scaffolds.

## Build

```sh
cmake -S . -B build
cmake --build build
```

Tests and benchmark scaffolds are enabled by default. Disable either target
with `-DMATOPS_BUILD_TESTS=OFF` or `-DMATOPS_BUILD_BENCHMARKS=OFF`.

Run the executable with:

```sh
./build/matops
```

See `docs/architecture.md` for the implementation boundaries and open design
decisions.
