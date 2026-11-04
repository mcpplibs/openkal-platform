#include <cstdio>
#include <thread>

// Volatile accesses keep the TLS operations present under LTO, including the
// main thread's read after a worker changes its own instance.
thread_local volatile int tls_value = 7;

int main() {
    int worker_value = 0;
    std::thread worker([&] {
        tls_value = 9;
        worker_value = tls_value;
    });
    worker.join();
    if (worker_value != 9 || tls_value != 7) return 1;
    std::puts("LTO TLS ok");
}
