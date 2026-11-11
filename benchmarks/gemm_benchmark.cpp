#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>

#include "gemm/gemm_config.hpp"
#include "matops/gemm.hpp"
#include "matops/matrix.hpp"

namespace {

using clock_type = std::chrono::steady_clock;

struct benchmark_case {
  const char* name;
  matops::index M;
  matops::index N;
  matops::index K;
};

bool run_case(const benchmark_case& test_case) {
  using matops::index;
  constexpr double alpha = 1.0;
  constexpr double beta = 0.0;
  constexpr int warmup_runs = 2;
  constexpr int sample_count = 7;

  matops::matrix<double> A(test_case.M, test_case.K);
  matops::matrix<double> B(test_case.K, test_case.N);
  matops::matrix<double> C(test_case.M, test_case.N);
  auto a = A.view();
  auto b = B.view();
  auto c = C.view();

  for (index i = 0; i < test_case.M; ++i) {
    for (index k = 0; k < test_case.K; ++k) {
      a(i, k) = static_cast<double>((i * 7 + k * 3) % 29 - 14) / 29.0;
    }
  }
  for (index k = 0; k < test_case.K; ++k) {
    for (index j = 0; j < test_case.N; ++j) {
      b(k, j) = static_cast<double>((k * 5 + j * 11) % 31 - 15) / 31.0;
    }
  }

  std::vector<double> reference(
      static_cast<std::size_t>(test_case.M * test_case.N));
  for (index i = 0; i < test_case.M; ++i) {
    for (index j = 0; j < test_case.N; ++j) {
      double sum = 0.0;
      for (index k = 0; k < test_case.K; ++k) {
        sum += a(i, k) * b(k, j);
      }
      reference[static_cast<std::size_t>(i * test_case.N + j)] =
          alpha * sum;
    }
  }

  const auto A_view = std::as_const(A).view();
  const auto B_view = std::as_const(B).view();
  for (int i = 0; i < warmup_runs; ++i) {
    matops::gemm(alpha, A_view, B_view, beta, c);
  }

  for (index i = 0; i < test_case.M; ++i) {
    for (index j = 0; j < test_case.N; ++j) {
      const double expected =
          reference[static_cast<std::size_t>(i * test_case.N + j)];
      const double tolerance = 1e-10 * std::max(1.0, std::abs(expected));
      if (std::abs(c(i, j) - expected) > tolerance) {
        std::cerr << "Correctness check failed for " << test_case.name
                  << " at C(" << i << ", " << j << "): got " << c(i, j)
                  << ", expected " << expected << '\n';
        return false;
      }
    }
  }

  std::array<double, sample_count> samples_ms{};
  for (int sample = 0; sample < sample_count; ++sample) {
    const auto start = clock_type::now();
    matops::gemm(alpha, A_view, B_view, beta, c);
    const auto finish = clock_type::now();
    samples_ms[static_cast<std::size_t>(sample)] =
        std::chrono::duration<double, std::milli>(finish - start).count();
  }
  std::sort(samples_ms.begin(), samples_ms.end());

  const double median_ms = samples_ms[sample_count / 2];
  const double operations = 2.0 * test_case.M * test_case.N * test_case.K;
  const double gflops = operations / (median_ms * 1.0e6);
  const double checksum = std::accumulate(c.data(), c.data() + test_case.M *
                                                        test_case.N,
                                          0.0);

  std::cout << std::left << std::setw(14) << test_case.name << std::right
            << std::setw(6) << test_case.M << 'x' << std::setw(6)
            << test_case.N << 'x' << std::setw(6) << test_case.K << "  "
            << std::setw(10) << std::fixed << std::setprecision(3) << median_ms
            << " ms  " << std::setw(9) << std::setprecision(3) << gflops
            << " GFLOP/s  checksum=" << std::setprecision(6) << checksum
            << '\n';
  return true;
}

}  // namespace

int main() {
  using matops::detail::gemm_config::KC;
  using matops::detail::gemm_config::MC;
  using matops::detail::gemm_config::NC;

  const std::array cases = {
      benchmark_case{"small", 16, 16, 16},
      benchmark_case{"square-64", 64, 64, 64},
      benchmark_case{"square-128", 128, 128, 128},
      benchmark_case{"square-256", 256, 256, 256},
      benchmark_case{"square-512", 512, 512, 512},
      benchmark_case{"tall", 128, 32, 64},
      benchmark_case{"wide", 32, 128, 64},
      benchmark_case{"M-edge", MC + 1, 5, 17},
      benchmark_case{"N-edge", 5, NC + 1, 17},
      benchmark_case{"K-edge", 5, 7, KC + 1},
  };

  std::cout << "GEMM benchmark (double; 2 warmups, median of 7 samples)\n"
            << "Shape: M x N x K\n";
  for (const auto& test_case : cases) {
    if (!run_case(test_case)) {
      return 1;
    }
  }
  return 0;
}
