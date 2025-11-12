#include "hypervisor.hpp"
#include <iostream>
#include <chrono>

using namespace std::chrono_literals;

Hypervisor::Hypervisor() : running_(false) {}

Hypervisor::~Hypervisor() {
    stop();
}

int Hypervisor::create_partition(const PartitionConfig& cfg, std::function<void(int)> entry) {
    auto p = std::make_unique<Partition>();
    p->cfg = cfg;
    p->entry = entry;
    p->should_run = false;
    int id = cfg.id;
    partitions_.push_back(std::move(p));
    return id;
}

void Hypervisor::start() {
    running_ = true;
    // start partition threads (they will wait until scheduled)
    for (auto &p_ptr : partitions_) {
        auto p = p_ptr.get();
        p->th = std::thread([this, p]() {
            // each partition calls its entry with the partition id
            p->entry(p->cfg.id);
        });
    }
    // start scheduler
    scheduler_thread_ = std::thread(&Hypervisor::scheduler_loop, this);
}

void Hypervisor::stop() {
    running_ = false;
    if (scheduler_thread_.joinable()) scheduler_thread_.join();
    for (auto &p_ptr : partitions_) {
        auto p = p_ptr.get();
        if (p->th.joinable()) p->th.join();
    }
}

void Hypervisor::set_cpu_quota(int partition_id, int quota_ms) {
    std::lock_guard<std::mutex> g(global_mtx_);
    for (auto &p_ptr : partitions_) {
        if (p_ptr->cfg.id == partition_id) {
            p_ptr->cfg.cpu_quota_ms = quota_ms;
            return;
        }
    }
}

void Hypervisor::send_message(const Message& m) {
    std::lock_guard<std::mutex> g(global_mtx_);
    for (auto &p_ptr : partitions_) {
        if (p_ptr->cfg.id == m.to_partition) {
            std::lock_guard<std::mutex> lg(p_ptr->mtx);
            p_ptr->inbox.push(m);
            p_ptr->cv.notify_one();
            return;
        }
    }
}

bool Hypervisor::try_receive(int partition_id, Message &out) {
    std::lock_guard<std::mutex> g(global_mtx_);
    for (auto &p_ptr : partitions_) {
        if (p_ptr->cfg.id == partition_id) {
            std::lock_guard<std::mutex> lg(p_ptr->mtx);
            if (p_ptr->inbox.empty()) return false;
            out = p_ptr->inbox.front();
            p_ptr->inbox.pop();
            return true;
        }
    }
    return false;
}

void Hypervisor::scheduler_loop() {
    using namespace std::chrono;
    while (running_) {
        std::vector<Partition*> snapshot;
        {
            std::lock_guard<std::mutex> g(global_mtx_);
            for (auto &p_ptr : partitions_) snapshot.push_back(p_ptr.get());
        }
        // simple round-robin with quotas
        for (auto *p : snapshot) {
            if (!running_) break;
            int quota = p->cfg.cpu_quota_ms;
            if (quota <= 0) quota = 10; // default
            // "grant" running: notify the partition if it's waiting on CV
            {
                std::unique_lock<std::mutex> lg(p->mtx);
                p->should_run = true;
                p->cv.notify_one();
            }
            std::this_thread::sleep_for(milliseconds(quota));
            {
                std::unique_lock<std::mutex> lg(p->mtx);
                p->should_run = false;
            }
        }
        std::this_thread::sleep_for(5ms);
    }
}
