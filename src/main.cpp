#include "hypervisor.hpp"
#include "rtos.hpp"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    Hypervisor hv;

    // Create two partitions (like two guests). IDs 1 and 2.
    PartitionConfig p1{1, 30, 1024*1024, "guest-A"};
    PartitionConfig p2{2, 20, 1024*1024, "guest-B"};

    // Entry function for partition: builds its RTOS and tasks
    auto entry = [&hv](int pid) {
        ToyRTOS rtos(&hv, pid);

        // simple periodic task
        rtos.create_task("heartbeat", [pid, &hv]() {
            static int counter = 0;
            counter++;
            std::cout << "[Partition " << pid << "] heartbeat " << counter << std::endl;
            // send a message to other partition occasionally
            if (pid == 1 && (counter % 5) == 0) {
                Message m; m.from_partition = 1; m.to_partition = 2; m.payload = "ping";
                hv.send_message(m);
            }
            if (pid == 2 && (counter % 7) == 0) {
                Message m; m.from_partition = 2; m.to_partition = 1; m.payload = "pong";
                hv.send_message(m);
            }
        }, 100);

        // another task with higher rate (shorter period)
        rtos.create_task("fast", [pid]() {
            // busy work to demonstrate scheduling fairness
            volatile int x = 0;
            for (int i=0;i<10000;i++) x += i;
            if (pid==1) std::cout << "[Partition 1] fast task did work\n";
        }, 40);

        rtos.start();
    };

    hv.create_partition(p1, entry);
    hv.create_partition(p2, entry);

    hv.start();

    // Let the simulation run for some time
    std::this_thread::sleep_for(std::chrono::seconds(6));

    std::cout << "Changing CPU quotas: give more time to partition 2\n";
    hv.set_cpu_quota(2, 50); // bump p2 quota

    std::this_thread::sleep_for(std::chrono::seconds(6));

    hv.stop();
    std::cout << "Simulation stopped.\n";
    return 0;
}
