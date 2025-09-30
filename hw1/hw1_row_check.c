#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#define REPEAT 1000

static inline void clflush(volatile void *p) {
    asm volatile("clflush (%0)" : : "r" (p));
}

static inline uint64_t rdtsc() {
    unsigned long a, d;
    asm volatile("rdtsc" : "=a" (a), "=d" (d));
    return a | ((uint64_t) d << 32);
}

static inline void memtest(int bytesy) {
    int bytes = 1<<19;
    uint64_t *ticks = (uint64_t *) aligned_alloc(64, REPEAT * sizeof(uint64_t));
    char *lineBuffer = (char *) aligned_alloc(64, bytes);

    
    // Initialize buffer
    for (int i = 0; i < bytes; i++) {
        lineBuffer[i] = '1';
    }
    long long sumy = 0;
    // Gather timing data
    for (int rep = 0; rep < REPEAT; rep++) {
        // INSERT_YOUR_CODE
        int rand_index = rand() % bytes;
        char *random_addr = lineBuffer + rand_index;
        asm volatile("lfence" ::: "memory"); // serialize following code
        uint64_t start = rdtsc();
        sumy += *random_addr;
        uint64_t end = rdtsc();
        asm volatile("lfence" ::: "memory"); // serialize following code
        
        clflush(random_addr);
        asm volatile("mfence" ::: "memory");  // Wait for flushes to complete
        

        ticks[rep] = end - start;
    }
    printf("Sumy: %lld\n", sumy);
    // INSERT_YOUR_CODE
    printf("First 30 ticks values:\n");
    int print_count = REPEAT < 30 ? REPEAT : 30;
    for (int i = 0; i < print_count; i++) {
        printf("%llu\n", (unsigned long long)ticks[i]);
    }
    // Calculate average
    uint64_t sum = 0;
    for (int i = 0; i < REPEAT; i++) {
        sum += ticks[i];
    }
    double average = (double)sum / REPEAT;
    printf("Average ticks: %.2f\n", average);

    // Save ticks to CSV file
    FILE *csv_file = fopen("random_row_check.csv", "w");
    if (csv_file != NULL) {
        for (int i = 0; i < REPEAT; i++) {
            fprintf(csv_file, "%llu\n", (unsigned long long)ticks[i]);
        }
        fclose(csv_file);
        printf("Ticks saved to ticks.csv\n");
    } else {
        printf("Failed to open ticks.csv for writing.\n");
    }
    // INSERT_YOUR_CODE
    sleep(1);
    sumy = 0;
    for (int rep = 0; rep < REPEAT; rep++) {
        // INSERT_YOUR_CODE
        asm volatile("lfence" ::: "memory"); // serialize following code
        uint64_t start = rdtsc();
        sumy += lineBuffer[rep];
        uint64_t end = rdtsc();
        asm volatile("lfence" ::: "memory"); // serialize following code
        
        clflush(lineBuffer + rep);
        asm volatile("mfence" ::: "memory");  // Wait for flushes to complete
        

        ticks[rep] = end - start;
    }
    printf("Sumy: %lld\n", sumy);
    // INSERT_YOUR_CODE
    // Print the first 30 values of ticks
    printf("First 30 ticks values:\n");
    print_count = REPEAT < 30 ? REPEAT : 30;
    for (int i = 0; i < print_count; i++) {
        printf("%llu\n", (unsigned long long)ticks[i]);
    }

    // Calculate average
    sum = 0;
    for (int i = 0; i < REPEAT; i++) {
        sum += ticks[i];
    }
    average = (double)sum / REPEAT;
    printf("Average ticks: %.2f\n", average);

    // Save ticks to CSV file
    csv_file = fopen("fixed_row_check.csv", "w");
    if (csv_file != NULL) {
        for (int i = 0; i < REPEAT; i++) {
            fprintf(csv_file, "%llu\n", (unsigned long long)ticks[i]);
        }
        fclose(csv_file);
        printf("Ticks saved to fixed_row_check.csv\n");
    } else {
        printf("Failed to open fixed_row_check.csv for writing.\n");
    }


    
    
    free(ticks);
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
