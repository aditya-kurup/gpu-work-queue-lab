#pragma once

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace gpu_lab {

struct Job {
  std::uint64_t id;
  int priority;
  std::size_t bytes;
  std::function<void()> execute;
};

struct Metrics {
  std::size_t submitted = 0;
  std::size_t completed = 0;
  std::size_t rejected = 0;
  std::size_t peak_bytes = 0;
  std::size_t in_flight_bytes = 0;
};

class Scheduler {
 public:
  Scheduler(std::size_t workers, std::size_t memory_budget);
  ~Scheduler();

  Scheduler(const Scheduler&) = delete;
  Scheduler& operator=(const Scheduler&) = delete;

  bool submit(Job job);
  void wait_idle();
  void stop();
  Metrics metrics() const;

 private:
  struct HigherPriority {
    bool operator()(const Job& left, const Job& right) const;
  };

  void worker_loop();

  const std::size_t memory_budget_;
  mutable std::mutex mutex_;
  std::condition_variable work_available_;
  std::condition_variable state_changed_;
  std::priority_queue<Job, std::vector<Job>, HigherPriority> queue_;
  std::vector<std::thread> workers_;
  Metrics metrics_;
  std::size_t active_jobs_ = 0;
  bool stopping_ = false;
};

}  // namespace gpu_lab
