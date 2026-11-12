#!/usr/bin/env python3
"""Sweep GEMM cache-block sizes using the Release benchmark suite."""

from __future__ import annotations

import argparse
import csv
import itertools
import math
import re
import statistics
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SCORE_CASES = (
    "square-64",
    "square-128",
    "square-256",
    "square-512",
    "tall",
    "wide",
)
BENCHMARK_LINE = re.compile(
    r"^\s*(?P<case>\S+)\s+(?P<M>\d+)x\s*(?P<N>\d+)x\s*"
    r"(?P<K>\d+)\s+(?P<ms>[\d.]+)\s+ms\s+"
    r"(?P<gflops>[\d.]+)\s+GFLOP/s"
)


def run(command: list[str]) -> str:
    result = subprocess.run(
        command,
        cwd=ROOT,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    if result.returncode != 0:
        raise RuntimeError(
            f"Command failed ({result.returncode}): {' '.join(command)}\n"
            f"{result.stdout}"
        )
    return result.stdout


def configure_and_build(build_dir: Path, mc: int, nc: int, kc: int,
                        parallel: int) -> Path:
    run(
        [
            "cmake",
            "-S",
            str(ROOT),
            "-B",
            str(build_dir),
            "-DCMAKE_BUILD_TYPE=Release",
            "-DMATOPS_BUILD_TESTS=OFF",
            "-DMATOPS_BUILD_BENCHMARKS=ON",
            f"-DMATOPS_GEMM_MC={mc}",
            f"-DMATOPS_GEMM_NC={nc}",
            f"-DMATOPS_GEMM_KC={kc}",
        ]
    )
    run(
        [
            "cmake",
            "--build",
            str(build_dir),
            "--config",
            "Release",
            "--target",
            "matops_benchmarks",
            "--parallel",
            str(parallel),
        ]
    )

    executable = build_dir / "matops_benchmarks"
    if sys.platform == "win32":
        multi_config_executable = build_dir / "Release" / "matops_benchmarks.exe"
        if multi_config_executable.exists():
            executable = multi_config_executable
        else:
            executable = build_dir / "matops_benchmarks.exe"
    return executable


def run_benchmark(executable: Path) -> dict[str, dict[str, float]]:
    output = run([str(executable)])
    results: dict[str, dict[str, float]] = {}
    for line in output.splitlines():
        match = BENCHMARK_LINE.match(line)
        if match:
            results[match.group("case")] = {
                "median_ms": float(match.group("ms")),
                "gflops": float(match.group("gflops")),
            }

    missing = set(SCORE_CASES) - results.keys()
    if missing:
        raise RuntimeError(
            "Benchmark output was missing cases: " + ", ".join(sorted(missing))
        )
    return results


def geometric_mean(results: dict[str, dict[str, float]]) -> float:
    return math.exp(
        sum(math.log(results[name]["gflops"]) for name in SCORE_CASES)
        / len(SCORE_CASES)
    )


def write_csv(path: Path, rows: list[dict[str, object]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fields = ["MC", "NC", "KC", "phase", "repeat", "score_gflops"]
    for case in SCORE_CASES:
        fields.extend((f"{case}_ms", f"{case}_gflops"))
    with path.open("w", newline="", encoding="utf-8") as output_file:
        writer = csv.DictWriter(output_file, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)


def result_row(
    mc: int,
    nc: int,
    kc: int,
    phase: str,
    repeat: int,
    results: dict[str, dict[str, float]],
) -> dict[str, object]:
    row: dict[str, object] = {
        "MC": mc,
        "NC": nc,
        "KC": kc,
        "phase": phase,
        "repeat": repeat,
        "score_gflops": geometric_mean(results),
    }
    for case in SCORE_CASES:
        row[f"{case}_ms"] = results[case]["median_ms"]
        row[f"{case}_gflops"] = results[case]["gflops"]
    return row


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mc", nargs="+", type=int, default=[32, 64, 128])
    parser.add_argument("--nc", nargs="+", type=int, default=[256, 512, 1024])
    parser.add_argument("--kc", nargs="+", type=int, default=[128, 256, 512])
    parser.add_argument(
        "--build-dir", type=Path, default=ROOT / "build" / "tune-release"
    )
    parser.add_argument(
        "--results",
        type=Path,
        default=ROOT / "build" / "gemm_block_tuning.csv",
    )
    parser.add_argument("--confirm-top", type=int, default=3)
    parser.add_argument("--confirm-runs", type=int, default=5)
    parser.add_argument("--parallel", type=int, default=2)
    args = parser.parse_args()

    candidates = list(itertools.product(args.mc, args.nc, args.kc))
    rows: list[dict[str, object]] = []
    sweep_results: list[tuple[float, int, int, int]] = []

    for number, (mc, nc, kc) in enumerate(candidates, start=1):
        print(
            f"[{number}/{len(candidates)}] Release sweep MC={mc} NC={nc} KC={kc}",
            flush=True,
        )
        executable = configure_and_build(args.build_dir, mc, nc, kc, args.parallel)
        results = run_benchmark(executable)
        score = geometric_mean(results)
        sweep_results.append((score, mc, nc, kc))
        rows.append(result_row(mc, nc, kc, "sweep", 1, results))
        print(f"  geometric-mean throughput: {score:.3f} GFLOP/s", flush=True)

    leaders = sorted(sweep_results, reverse=True)[: max(1, args.confirm_top)]
    leader_configs = [(mc, nc, kc) for _, mc, nc, kc in leaders]
    baseline = (64, 512, 256)
    if baseline not in leader_configs:
        leader_configs.append(baseline)

    confirmed: list[tuple[float, int, int, int]] = []
    for mc, nc, kc in leader_configs:
        print(f"Confirming MC={mc} NC={nc} KC={kc}", flush=True)
        executable = configure_and_build(args.build_dir, mc, nc, kc, args.parallel)
        repeat_scores = []
        for repeat in range(1, args.confirm_runs + 1):
            results = run_benchmark(executable)
            repeat_scores.append(geometric_mean(results))
            rows.append(result_row(mc, nc, kc, "confirm", repeat, results))
        final_score = statistics.median(repeat_scores)
        confirmed.append((final_score, mc, nc, kc))

    write_csv(args.results, rows)
    print("\nConfirmed Release ranking (geometric mean GFLOP/s):")
    for score, mc, nc, kc in sorted(confirmed, reverse=True):
        print(f"  MC={mc:3d} NC={nc:4d} KC={kc:3d}: {score:.3f}")

    best_score, best_mc, best_nc, best_kc = max(confirmed)
    print(
        f"\nSelected candidate: MC={best_mc} NC={best_nc} KC={best_kc} "
        f"(confirmed score {best_score:.3f} GFLOP/s)"
    )
    print(f"Detailed results: {args.results}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, RuntimeError, subprocess.CalledProcessError) as error:
        print(error, file=sys.stderr)
        raise SystemExit(1)
