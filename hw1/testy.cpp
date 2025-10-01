#include <iostream>
#include <ctime>
#include <unistd.h>   // for sleep()
#include <stdlib.h>
#include <stdint.h>
inline uint64_t rdtsc() {
    unsigned long a, d;
    asm volatile("rdtscp" : "=a" (a), "=d" (d) :: "rcx");
    return a | ((uint64_t) d << 32);
}

int main() {
    // Warm up
    volatile uint64_t dummy = rdtsc();

    // Take first readings
    uint64_t start_ticks = rdtsc();
    timespec ts_start;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    // Sleep ~1 second (you can shorten this if you like)
    sleep(1);

    // Take end readings
    uint64_t end_ticks = rdtsc();
    timespec ts_end;
    clock_gettime(CLOCK_MONOTONIC, &ts_end);

    // Compute elapsed seconds
    double elapsed_s =
        (ts_end.tv_sec - ts_start.tv_sec) +
        (ts_end.tv_nsec - ts_start.tv_nsec) / 1e9;

    uint64_t ticks = end_ticks - start_ticks;
    double ticks_per_sec = ticks / elapsed_s;

    std::cout << "Ticks measured: " << ticks << "\n";
    std::cout << "Elapsed time (s): " << elapsed_s << "\n";
    std::cout << "Ticks per second: " << ticks_per_sec << "\n";

    return 0;
}