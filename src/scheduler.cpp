#include "scheduler.hpp"

#include <stdexcept>

namespace gpu_lab {

bool Scheduler::HigherPriority::operator()(const Job& left, const Job& right) const {
  if (left.priority != right.priority) return left.priority < right.priority;
  return left.id > right.id;
}

Scheduler::Scheduler(std::size_t workers, std::size_t memory_budget)
    : memory_budget_(memory_budget) {
  if (workers == 0) throw std::invalid_argument("workers must be positive");
  if (memory_budget == 0) throw std::invalid_argument("memory budget must be positive");
  workers_.reserve(workers);
  for (std::size_t i = 0; i < workers; ++i) workers_.emplace_back(&Scheduler::worker_loop, this);
}

Scheduler::~Scheduler() { stop(); }

bool Scheduler::submit(Job job) {
  if (!job.execute || job.bytes == 0 || job.bytes > memory_budget_) {
    std::lock_guard<std::mutex> lock(mutex_);
    ++metrics_.rejected;
    return false;
  }
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (stopping_) {
      ++metrics_.rejected;
      return false;
    }
    queue_.push(std::move(job));
    ++metrics_.submitted;
  }
  work_available_.notify_one();
  return true;
}

void Scheduler::worker_loop() {
  while (true) {
    Job job{};
    {
      std::unique_lock<std::mutex> lock(mutex_);
      work_available_.wait(lock, [this] {
        return stopping_ || (!queue_.empty() &&
                             metrics_.in_flight_bytes + queue_.top().bytes <= memory_budget_);
      });
      if (stopping_ && queue_.empty()) return;
      if (queue_.empty() || metrics_.in_flight_bytes + queue_.top().bytes > memory_budget_) {
        continue;
      }
      job = queue_.top();
      queue_.pop();
      ++active_jobs_;
      metrics_.in_flight_bytes += job.bytes;
      if (metrics_.in_flight_bytes > metrics_.peak_bytes) {
        metrics_.peak_bytes = metrics_.in_flight_bytes;
      }
    }

    try {
      job.execute();
    } catch (...) {
      // A failed job still releases its admission reservation.
    }

    {
      std::lock_guard<std::mutex> lock(mutex_);
      --active_jobs_;
      metrics_.in_flight_bytes -= job.bytes;
      ++metrics_.completed;
      if (queue_.empty() && active_jobs_ == 0) state_changed_.notify_all();
    }
    work_available_.notify_all();
  }
}

void Scheduler::wait_idle() {
  std::unique_lock<std::mutex> lock(mutex_);
  state_changed_.wait(lock, [this] { return queue_.empty() && active_jobs_ == 0; });
}

void Scheduler::stop() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (stopping_) return;
    stopping_ = true;
  }
  work_available_.notify_all();
  for (auto& worker : workers_) {
    if (worker.joinable()) worker.join();
  }
}

Metrics Scheduler::metrics() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return metrics_;
}

}  // namespace gpu_lab
