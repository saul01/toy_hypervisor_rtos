#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <atomic>
#include <cstdint>
#include <string>

struct PartitionConfig {
    int id;
    int cpu_quota_ms; // scheduling quota per round
    size_t memory_quota_bytes;
    std::string name;
};

struct Message {
    int from_partition;
    int to_partition;
    std::string payload;
};

class Hypervisor {
public:
    Hypervisor();
    ~Hypervisor();

    int create_partition(const PartitionConfig& cfg, std::function<void(int)> entry);
    void start();
    void stop();

    // scheduling control
    void set_cpu_quota(int partition_id, int quota_ms);
    void send_message(const Message& m);
    bool try_receive(int partition_id, Message &out);

private:
    struct Partition {
        PartitionConfig cfg;
        std::function<void(int)> entry;
        std::thread th;
        std::atomic<bool> should_run;
        std::mutex mtx;
        std::condition_variable cv;
        std::queue<Message> inbox;
    };

    std::vector<std::unique_ptr<Partition>> partitions_;
    std::mutex global_mtx_;
    std::atomic<bool> running_;
    std::thread scheduler_thread_;

    void scheduler_loop();
};
