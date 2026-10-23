#include "scheduler.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

int main(int argc, char** argv) {
  const std::size_t jobs = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 100;
  gpu_lab::Scheduler scheduler(4, 1 << 20);
  for (std::size_t i = 0; i < jobs; ++i) {
    const int priority = static_cast<int>(i % 4);
    const std::size_t bytes = 16 * 1024 + (i % 8) * 4096;
    scheduler.submit({i, priority, bytes, [i] {
      std::this_thread::sleep_for(std::chrono::microseconds(100 + (i % 5) * 20));
    }});
  }
  scheduler.wait_idle();
  const auto stats = scheduler.metrics();
  std::cout << "submitted=" << stats.submitted << " completed=" << stats.completed
            << " rejected=" << stats.rejected << " peak_bytes=" << stats.peak_bytes << '\n';
}
