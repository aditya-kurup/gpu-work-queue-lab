#include "scheduler.hpp"

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <thread>

namespace {
void require(bool condition) {
  if (!condition) std::abort();
}
}  // namespace

int main() {
  gpu_lab::Scheduler scheduler(3, 100);
  std::atomic<int> ran{0};
  require(!scheduler.submit({1, 0, 101, [&] { ++ran; }}));
  for (std::uint64_t id = 0; id < 20; ++id) {
    require(scheduler.submit({id, static_cast<int>(id % 3), 25, [&] {
      std::this_thread::sleep_for(std::chrono::microseconds(100));
      ++ran;
    }}));
  }
  scheduler.wait_idle();
  const auto stats = scheduler.metrics();
  require(ran == 20);
  require(stats.submitted == 20);
  require(stats.completed == 20);
  require(stats.rejected == 1);
  require(stats.in_flight_bytes == 0);
  require(stats.peak_bytes <= 75);
}
