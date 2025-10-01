#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define REPEAT 100000

inline void clflush(volatile void *p) {
    asm volatile("clflush (%0)" : : "r" (p));
}

inline uint64_t rdtsc() {
    unsigned long a, d;
    asm volatile("rdtscp" : "=a" (a), "=d" (d));
    return a | ((uint64_t) d << 32);
}

inline void memtest(int bytes) {
    uint64_t *firsts = (uint64_t *) aligned_alloc(64, REPEAT * sizeof(uint64_t));
    uint64_t *seconds = (uint64_t *) aligned_alloc(64, REPEAT * sizeof(uint64_t));
    char *lineBuffer = (char *) aligned_alloc(64, 1<<30);
    
    // Initialize buffer
    for (int i = 0; i < 1<<30; i++) {
        lineBuffer[i] = '1';
    }
    int sumy = 0;
    // Gather timing data

    for (int rep = 0; rep < REPEAT; rep++) {
        // INSERT_YOUR_CODE
        // Pull two random numbers in the buffer for the copying
        

        char* tmp1 = lineBuffer + (rand() % ((1<<23)-(bytes/64)))*64;
        for(int i = 0; i < 2; i+=1) {
            if (i == 1){
                tmp1 = tmp1 + (1<<2);
            }
            clflush(tmp1);
            

            asm volatile("lfence" ::: "memory"); // serialize following code
            uint64_t start = rdtsc();
            
            long long x = tmp1[0];
            asm volatile("lfence" ::: "memory"); // serialize following code
            uint64_t end = rdtsc();
            asm volatile("mfence" ::: "memory");  // Wait for flushes to complete
            if(i == 0) {
                firsts[rep] = end - start;
            }
            else {
                seconds[rep] = end - start;
            }
            sumy += x;
        }

    }
    printf("Sumy: %d\n", sumy);
    
    // Calculate initial average and standard deviation (with all data)
    uint64_t sum = 0;
    for (int i = 0; i < REPEAT; i++) {
        sum += firsts[i];
    }
    double average_with_outliers = (double) sum / REPEAT;

    printf("Average first: %f\n", average_with_outliers);
    // computing standard deviation
    double sum_of_squares = 0;
    for (int i = 0; i < REPEAT; i++) {
        sum_of_squares += (firsts[i] - average_with_outliers) * (firsts[i] - average_with_outliers);
    }
    double standard_deviation = sqrt(sum_of_squares / REPEAT);
    printf("Standard deviation first: %f\n", standard_deviation);
    sum = 0;
    for (int i = 0; i < REPEAT; i++) {
        sum += seconds[i];
    }
    average_with_outliers = (double) sum / REPEAT;
    printf("Average second: %f\n", average_with_outliers);
    sum_of_squares = 0;
    for (int i = 0; i < REPEAT; i++) {
        sum_of_squares += (seconds[i] - average_with_outliers) * (seconds[i] - average_with_outliers);
    }
    standard_deviation = sqrt(sum_of_squares / REPEAT);
    printf("Standard deviation second: %f\n", standard_deviation);
    //print first 10 values of firsts
    for (int i = 0; i < 10; i++) {
        printf("%llu\n", (unsigned long long)firsts[i]);
    }
    printf("First 10 values of firsts:\n");
    //print first 10 values of seconds
    for (int i = 0; i < 10; i++) {
        printf("%llu\n", (unsigned long long)seconds[i]);
    }
    printf("First 10 values of seconds:\n");
    
    
    free(firsts);
    free(seconds);
    free(lineBuffer);
}

int main(int ac, char **av) {
    if (ac != 2) {
        printf("Usage: %s <x> (copies 2^x bytes)\n", av[0]);
        return 1;
    }
    
    int x = atoi(av[1]);
    int bytes = 1 << x;  // 2^x
    
    printf("--------------------------------\n");
    memtest(bytes);
    return 0;
}
