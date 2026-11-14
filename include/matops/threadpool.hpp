#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace matops {

class Threadpool {
public:
  explicit Threadpool(
      std::size_t thread_count = std::thread::hardware_concurrency()) {
    if (thread_count == 0) {
      thread_count = 1;
    }

    workers_.reserve(thread_count);
    try {
      for (std::size_t i = 0; i < thread_count; ++i) {
        workers_.emplace_back([this] { worker_loop(); });
      }
    } catch (...) {
      stop_workers();
      throw;
    }
  }

  Threadpool(const Threadpool&) = delete;
  Threadpool& operator=(const Threadpool&) = delete;
  Threadpool(Threadpool&&) = delete;
  Threadpool& operator=(Threadpool&&) = delete;

  ~Threadpool() { stop_workers(); }

  [[nodiscard]] std::size_t size() const noexcept { return workers_.size(); }

  template <class F, class... Args>
  auto enqueue(F&& f, Args&&... args)
      -> std::future<std::invoke_result_t<F, Args...>> {
    using result_type = std::invoke_result_t<F, Args...>;
    auto bound_task =
        std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    auto task = std::make_shared<std::packaged_task<result_type()>>(
        std::move(bound_task));
    std::future<result_type> result = task->get_future();

    {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      if (stop_) {
        throw std::runtime_error("enqueue on stopped thread pool");
      }
      tasks_.emplace([task] { (*task)(); });
    }
    cv_.notify_one();
    return result;
  }

private:
  void worker_loop() {
    while (true) {
      std::function<void()> task;
      {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        cv_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
        if (stop_ && tasks_.empty()) {
          return;
        }
        task = std::move(tasks_.front());
        tasks_.pop();
      }
      task();
    }
  }

  void stop_workers() noexcept {
    {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      stop_ = true;
    }
    cv_.notify_all();
    for (std::thread& worker : workers_) {
      if (worker.joinable()) {
        worker.join();
      }
    }
  }

  std::vector<std::thread> workers_;
  std::queue<std::function<void()>> tasks_;
  std::mutex queue_mutex_;
  std::condition_variable cv_;
  bool stop_ = false;
};

}  // namespace matops
