# GEBP Design

## Purpose and boundary

GEBP is an internal building block for GEMM and other operations that need a
panel product. It is not part of the public `include/matops` API. Its initial
implementation should be a simple scalar reference kernel; packing,
microkernels, SIMD, and tuning can be added later without changing the
mathematical contract.

The proposed internal interface is:

```cpp
namespace matops::detail {

template <typename T>
void gebp(
    T alpha,
    matrix_view<const T> A_panel,
    matrix_view<const T> B_panel,
    matrix_view<T> C_tile);

}  // namespace matops::detail
```

The views carry the panel dimensions and strides. Keep the declaration and,
for a header-only template implementation, its definition under `src/gebp/`
(for example, `src/gebp/gebp.hpp`).

## Operation

For compatible panel shapes, one call performs:

```text
C_tile += alpha * A_panel * B_panel
```

For an `M x Kp` A panel and a `Kp x N` B panel, C is an `M x N` tile. A GEBP
call updates the supplied tile for the supplied K panel; it does not traverse
GEMM's full output or K dimensions.

GEBP has no `beta` argument and does not initialize or scale C. GEMM owns the
full `C = alpha * A * B + beta * C` operation: it applies `beta` to each C tile
once, then calls GEBP for successive K panels to accumulate the product. When
`beta` is zero, GEMM should set C to zero without reading its prior contents.
GEMM may skip GEBP when `alpha` or the K dimension is zero.

## Shapes and edge panels

The shape preconditions are:

- `A_panel.columns() == B_panel.rows()`
- `C_tile.rows() == A_panel.rows()`
- `C_tile.columns() == B_panel.columns()`

An incompatible shape is an internal caller contract violation. The public
GEMM entry point validates its arguments before forming panels; GEBP may use
debug assertions to catch incorrect internal calls.

Panel dimensions are variable. A caller passes the actual dimensions of each
panel, including smaller edge panels; GEBP does not require dimensions to be
multiples of a fixed tile size. If M or N is zero, or Kp is zero, GEBP leaves C
unchanged. If `alpha` is zero, GEBP also leaves C unchanged without reading A
or B.

## Layout and aliasing

Panels are passed as strided `matrix_view`s, not as a required packed format.
Input views may use any valid signed or zero strides, provided every addressed
element refers to valid storage. C may also use strided storage, provided its
logical elements occupy distinct locations.

C must not overlap either input view. A and B may overlap each other because
they are read-only. The caller is responsible for meeting these storage
preconditions; GEBP does not detect overlap.

An optimized implementation may add fast paths for layouts it supports and
use packing or a fallback for other valid strides. Those implementation
details do not change this interface contract.

## Types and numerical behavior

All views and `alpha` use the same element type `T`; the initial contract does
not include mixed-precision accumulation. The reference implementation should
be exercised with at least `float` and `double`.

Implementations need not produce bitwise-identical results. Different
accumulation orders or fused operations may yield different floating-point
rounding. Correctness tests should compare against a reference with
tolerances appropriate to the type and problem size.

## Execution model

GEBP is synchronous and performs its work on the calling thread. It does not
create workers or maintain shared mutable state. Callers may execute
independent GEBP calls concurrently when their output regions do not overlap;
the caller must synchronize calls that would update the same C elements.

## Implementation sequence

1. Implement a scalar reference GEBP that follows this contract.
2. Add focused GEBP correctness tests, including edge dimensions, supported
   strides, and accumulation into an existing C tile.
3. Integrate compiled implementation files into `CMakeLists.txt` when needed.
4. Implement GEMM's M/N/K-panel orchestration and test repeated K-panel
   accumulation against a reference GEMM.
5. Add reproducible benchmarks. Use their results to guide block sizes before
   considering packing, fixed-size microkernels, SIMD, dispatch, or tuning.

Runtime autotuning is not part of the initial design. Start with measured,
explicit parameters and revisit tuning only after there are multiple useful
kernel variants to select among.
