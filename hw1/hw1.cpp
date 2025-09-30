#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define REPEAT 1000000

inline void clflush(volatile void *p) {
    asm volatile("clflush (%0)" : : "r" (p));
}

inline uint64_t rdtsc() {
    unsigned long a, d;
    asm volatile("rdtsc" : "=a" (a), "=d" (d));
    return a | ((uint64_t) d << 32);
}

inline void memtest(int bytes) {
    uint64_t *ticks = (uint64_t *) aligned_alloc(64, REPEAT * sizeof(uint64_t));
    char *lineBuffer = (char *) aligned_alloc(64, bytes);
    char *lineBufferCopy = (char *) aligned_alloc(64, bytes);
    
    // Initialize buffer
    for (int i = 0; i < bytes; i++) {
        lineBuffer[i] = '1';
    }
    
    // Gather timing data
    for (int rep = 0; rep < REPEAT; rep++) {
        uint64_t start = rdtsc();
        memcpy(lineBufferCopy, lineBuffer, bytes);
        uint64_t end = rdtsc();
        
        clflush(lineBuffer);
        clflush(lineBufferCopy);
        asm volatile("mfence" ::: "memory");  // Wait for flushes to complete
        

        ticks[rep] = end - start;
    }
    
    // Calculate initial average and standard deviation (with all data)
    uint64_t sum = 0;
    for (int i = 0; i < REPEAT; i++) {
        sum += ticks[i];
    }
    double average_with_outliers = (double) sum / REPEAT;
    
    double variance_with_outliers = 0;
    for (int i = 0; i < REPEAT; i++) {
        double diff = (double) ticks[i] - average_with_outliers;
        variance_with_outliers += diff * diff;
    }
    variance_with_outliers /= REPEAT;
    double stdev_with_outliers = sqrt(variance_with_outliers);
    
    // Identify outliers (more than 2 standard deviations from mean)
    int *outlier_indices = (int *) malloc(REPEAT * sizeof(int));
    int outlier_count = 0;
    double threshold = 2.0 * stdev_with_outliers;
    
    for (int i = 0; i < REPEAT; i++) {
        if (fabs((double) ticks[i] - average_with_outliers) > threshold) {
            outlier_indices[outlier_count] = i;
            outlier_count++;
        }
    }
    
    // Calculate statistics without outliers
    int valid_count = REPEAT - outlier_count;
    uint64_t sum_without_outliers = 0;
    for (int i = 0; i < REPEAT; i++) {
        bool is_outlier = false;
        for (int j = 0; j < outlier_count; j++) {
            if (i == outlier_indices[j]) {
                is_outlier = true;
                break;
            }
        }
        if (!is_outlier) {
            sum_without_outliers += ticks[i];
        }
    }
    
    double average_without_outliers = (double) sum_without_outliers / valid_count;
    
    double variance_without_outliers = 0;
    for (int i = 0; i < REPEAT; i++) {
        bool is_outlier = false;
        for (int j = 0; j < outlier_count; j++) {
            if (i == outlier_indices[j]) {
                is_outlier = true;
                break;
            }
        }
        if (!is_outlier) {
            double diff = (double) ticks[i] - average_without_outliers;
            variance_without_outliers += diff * diff;
        }
    }
    variance_without_outliers /= valid_count;
    double stdev_without_outliers = sqrt(variance_without_outliers);
    
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
        bool is_outlier = false;
        for (int j = 0; j < outlier_count; j++) {
            if (i == outlier_indices[j]) {
                is_outlier = true;
                break;
            }
        }
        fprintf(csv_file, "%d,%llu,%s,%d\n", i, ticks[i], is_outlier ? "true" : "false", bytes);
    }
    
    fclose(csv_file);
    
    // Print results
    printf("Copying %d bytes (%d iterations):\n", bytes, REPEAT);
    printf("Data saved to: %s\n", csv_filename);
    printf("\n=== WITH ALL DATA (including outliers) ===\n");
    printf("Average: %.2f ticks\n", average_with_outliers);
    printf("Standard deviation: %.2f ticks\n", stdev_with_outliers);
    printf("Total: %llu ticks\n", sum);
    
    printf("\n=== OUTLIERS DETECTED ===\n");
    printf("Number of outliers: %d (%.2f%% of data)\n", outlier_count, (double) outlier_count / REPEAT * 100);
    if (outlier_count > 0) {
        printf("Outlier values: ");
        for (int i = 0; i < outlier_count; i++) {
            printf("%llu", ticks[outlier_indices[i]]);
            if (i < outlier_count - 1) printf(", ");
        }
        printf("\n");
    }
    
    printf("\n=== WITHOUT OUTLIERS ===\n");
    printf("Valid data points: %d\n", valid_count);
    printf("Average: %.2f ticks\n", average_without_outliers);
    printf("Standard deviation: %.2f ticks\n", stdev_without_outliers);
    printf("Total: %llu ticks\n", sum_without_outliers);
    
    free(ticks);
    free(lineBuffer);
    free(lineBufferCopy);
    free(outlier_indices);
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
