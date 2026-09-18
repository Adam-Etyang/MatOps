# MatOps Architecture

The implementation is intentionally staged around GEMM:

```text
matrix / matrix_view
        |
      gemm
        |
    dispatch
        |
reference -> blocked -> packed -> ISA-specific kernels
```

## Open Decisions

- Default storage layout and conversion policy
- Bounds-checking policy for debug and release builds
- Alignment and allocator strategy
- Cache blocking parameters (`MC`, `NC`, `KC`)
- Microkernel dimensions (`MR`, `NR`)
- CPU feature detection and dispatch mechanism
- Threading model and small-matrix cutoff
- Aliasing and overlapping-view semantics

The reference implementation should be completed before adding packing,
SIMD kernels, or threading. Each optimization should retain a correctness
comparison against the reference path.
