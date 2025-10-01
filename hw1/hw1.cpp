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
    uint64_t *ticks = (uint64_t *) aligned_alloc(64, REPEAT * sizeof(uint64_t));
    char *lineBuffer = (char *) aligned_alloc(64, 1<<30);
    char *lineBufferCopy = (char *) aligned_alloc(64, 1<<30);
    
    // Initialize buffer
    for (int i = 0; i < 1<<30; i++) {
        lineBuffer[i] = '1';
        lineBufferCopy[i] = '0';
    }
    int sumy = 0;
    // Gather timing data
    for (int rep = 0; rep < REPEAT; rep++) {
        // INSERT_YOUR_CODE
        // Pull two random numbers in the buffer for the copying
        
        
        char* tmp1 = lineBuffer + (rand() % ((1<<23)-(bytes/64)))*64;
        char* tmp2 = lineBufferCopy + (rand() % ((1<<23)-(bytes/64)))*64;
        for(int i = 0; i < bytes; i+=64) {
            clflush(tmp1 + i);
            clflush(tmp2 + i);
        }
        asm volatile("lfence" ::: "memory"); // serialize following code
        uint64_t start = rdtsc();
        
        memcpy(tmp2, tmp1, bytes);
        asm volatile("lfence" ::: "memory"); // serialize following code
        uint64_t end = rdtsc();
        asm volatile("mfence" ::: "memory");  // Wait for flushes to complete
        volatile char tmp = lineBufferCopy[rand() % bytes];
        sumy += tmp;
        

        ticks[rep] = end - start;
    }
    printf("Sumy: %d\n", sumy);
    
    // Calculate initial average and standard deviation (with all data)
    uint64_t sum = 0;
    for (int i = 0; i < REPEAT; i++) {
        sum += ticks[i];
    }
    double average_with_outliers = (double) sum / REPEAT;
    
    
    
    // Create CSV filename based on bytes copied
    char csv_filename[100];
    sprintf(csv_filename, "timing_data_%d_bytes.csv", bytes);
    
    // Write data to CSV file
    FILE *csv_file = fopen(csv_filename, "w");
    if (csv_file == NULL) {
        printf("Error: Could not create CSV file %s\n", csv_filename);
        return;
    }
    
    // Write CSV header
    fprintf(csv_file, "iteration,ticks,is_outlier,bytes_copied\n");
    
    // Write all data points
    for (int i = 0; i < REPEAT; i++) {
        fprintf(csv_file, "%d,%llu\n", i, ticks[i]);
    }
    
    fclose(csv_file);
    
    // Print results
    printf("Copying %d bytes (%d iterations):\n", bytes, REPEAT);
    printf("Data saved to: %s\n", csv_filename);
    printf("\n=== WITH ALL DATA (including outliers) ===\n");
    printf("Average: %.2f ticks\n", average_with_outliers);
    printf("Total: %llu ticks\n", sum);
    
    printf("\n=== OUTLIERS DETECTED ===\n");
    
    
    free(ticks);
    free(lineBuffer);
    free(lineBufferCopy);
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
