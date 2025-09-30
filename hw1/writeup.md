## Question 1

To collect the data, great pain was taken to try to get accurate results. Specifically, fighting against the prefetcher proved to be a bit of a challenge. When we were pulling from the same address over and over, even flushing the cache did not stop the prefetcher from pulling values from ram early, making all of the copyies below 2^9 or so take no time (becides the measurment overhead).

This was solved by creating buffers that were about a gigabyte in size, then copying a random subset of that range to a seperate random subset of the other range. This may not fully account for the write time still, but did get us much more reasonable numbers for loads.

Our final benchmarking code was as follows:
```c++
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
        // INSERT_YOUR_CODE
        // Use the lineBuffer in some way, e.g., sum its contents to prevent optimization
        // INSERT_YOUR_CODE
        // Read a random byte of lineBufferCopy to prevent optimization
        volatile char tmp = lineBufferCopy[rand() % bytes];
        sumy += tmp;
        

        ticks[rep] = end - start;
    }
    printf("Sumy: %d\n", sumy);
```

[[KRIS, PUT THESE SOMEWHERE NICE AND PULL THE FIGURES FROM THE NOTEBOOK]]

