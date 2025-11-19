# Toy Hypervisor + Toy RTOS

Simulates a hypervisor and a real-time OS (RTOS) in user-space.
Designed for learning — **not** a real hypervisor.

## Features
- Toy hypervisor that manages multiple *partitions* (guests) and dynamically assigns CPU quotas.
- Toy RTOS that runs inside each partition: simple preemptive scheduler (rate-monotonic style).
- Inter-partition message passing via a hypervisor-provided channel.
- Demonstrates concepts: partitioning, scheduling, memory quotas, context switching (simulated), and simple verification hooks.

## Requirements
- Ubuntu (x86_64) laptop.
- g++ (supporting C++17), make, python3 (for packaging).
- No root required — runs entirely in user-space.

## Build & Run
```bash
# build
make

# run
./bin/toy_hv_rt
```

## Structure
- `src/` - C++ source code (hypervisor, rtos, demo).
- `Makefile` - build rules.
- `test/` - simple input scenarios (not needed to run).

## Notes
This repo is intentionally small and well-documented. Use it to:
- Talk through OS/hypervisor.
- Modify schedulers, memory policies, and verification (e.g., simple assertions).

