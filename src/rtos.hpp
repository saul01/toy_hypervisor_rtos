#pragma once
#include "hypervisor.hpp"
#include <functional>
#include <vector>
#include <chrono>
#include <atomic>

// Simple RTOS primitives implemented inside a partition (guest).
class ToyRTOS {
public:
    ToyRTOS(Hypervisor* hv, int partition_id);
    ~ToyRTOS();

    using TaskFn = std::function<void()>;

    int create_task(const std::string &name, TaskFn fn, int period_ms);
    void start(); // start RTOS main loop (runs in partition thread)
    void stop();

    // yield back to hypervisor (simulated)
    void yield();

    // blocking receive for inter-partition messages
    bool recv_message(Message &m, int timeout_ms = 0);

private:
    Hypervisor* hv_;
    int pid_;
struct Task {
    std::string name;
    TaskFn fn;
    int period_ms;
    std::chrono::steady_clock::time_point next_run;
    std::atomic<bool> active;

    Task() : period_ms(0), active(false) {}
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    Task(Task&& other) noexcept
        : name(std::move(other.name)),
          fn(std::move(other.fn)),
          period_ms(other.period_ms),
          next_run(other.next_run),
          active(other.active.load()) {}
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            name = std::move(other.name);
            fn = std::move(other.fn);
            period_ms = other.period_ms;
            next_run = other.next_run;
            active.store(other.active.load());
        }
        return *this;
    }
};
    std::vector<Task> tasks_;
    std::atomic<bool> running_;
};
