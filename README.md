# GPU Work Queue Lab

A small C++17 systems lab that models the host side of a GPU command scheduler. It demonstrates priority scheduling, memory-budget admission, worker synchronization, backpressure, and observable execution metrics.

## Why this project

The design targets system software topics in the NVIDIA GPU/Tegra description: CPU and memory behavior, operating-system scheduling, parallel execution, C++ systems programming, and performance instrumentation. It complements a CUDA kernel project by showing how work can be admitted and coordinated before it reaches an accelerator.

## Build and test

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/work_queue_demo 1000
```

## Design notes

- A `std::priority_queue` dispatches higher-priority work first, with FIFO ordering inside a priority.
- Each job reserves a byte budget when a worker dequeues it. Oversized jobs are rejected before entering the queue.
- `condition_variable` wakeups coordinate producers, workers, and the idle barrier without polling.
- Every worker releases its reservation even when a job callback throws.
- Tests cover invalid admission, concurrent execution, idle detection, and the peak memory invariant.

## Interview discussion

The next extension would replace the callback with a CUDA stream submission and expose a completion event, while retaining the scheduler's admission and accounting layer. A Linux version could add CPU affinity and `perf` counters to compare queue latency under different worker counts.
