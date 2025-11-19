#include "rtos.hpp"
#include <iostream>
#include <thread>

using namespace std::chrono;

ToyRTOS::ToyRTOS(Hypervisor* hv, int partition_id) : hv_(hv), pid_(partition_id), running_(false) {}

ToyRTOS::~ToyRTOS() { stop(); }

int ToyRTOS::create_task(const std::string &name, TaskFn fn, int period_ms) {
    Task t;
    t.name = name;
    t.fn = fn;
    t.period_ms = period_ms;
    t.next_run = steady_clock::now() + milliseconds(period_ms);
    t.active = true;
    tasks_.push_back(std::move(t));
    return (int)tasks_.size()-1;
}

void ToyRTOS::start() {
    running_ = true;
    using namespace std::chrono;

    // Main RTOS loop: simple cooperative preemptive by checking time and hv notifications
    while (running_) {
        auto now = steady_clock::now();
        for (auto &t : tasks_) {
            if (!t.active) continue;
            if (now >= t.next_run) {
                // Run the task but cooperative: task can call yield() to let hypervisor run
                t.next_run = now + milliseconds(t.period_ms);
                t.fn();
            }
            // Check for messages (non-blocking)
            Message m;
            if (hv_->try_receive(pid_, m)) {
                std::cout << "[Partition " << pid_ << "] received message from " << m.from_partition << ": '" << m.payload << "'\n";
            }
        }
        // simulate low-power wait until hypervisor schedules this partition to run
        // We'll use condition_variable from hypervisor partition; but ToyRTOS does cooperative sleep
        std::this_thread::sleep_for(milliseconds(1));
        // yield control so hypervisor can preempt (simulated)
        yield();
    }
}

void ToyRTOS::stop() {
    running_ = false;
}

void ToyRTOS::yield() {
    // simulated yield: wait until hypervisor repaints CPU time for this partition
    // We locate the partition structure inside hypervisor by asking hv to notify (via try_receive mechanism)
    // For simplicity, spin-wait on a short sleep to allow scheduler to preempt
    std::this_thread::sleep_for(milliseconds(0));
}

bool ToyRTOS::recv_message(Message &m, int timeout_ms) {
    if (timeout_ms <= 0) {
        return hv_->try_receive(pid_, m);
    }
    auto deadline = steady_clock::now() + milliseconds(timeout_ms);
    while (steady_clock::now() < deadline) {
        if (hv_->try_receive(pid_, m)) return true;
        std::this_thread::sleep_for(milliseconds(1));
    }
    return false;
}
