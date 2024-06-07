#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "allocators/pool_allocator.h"

#define BENCHMARK_LENGTH 100000
#define NUM_AVERAGE 20
#define ALLOCATION_SIZE_BYTES 1024

int main(void) {
  puts("Starting memory allocator benchmark...");
  float start_time, finish_time, duration_ms;

  float malloc_average = 0, memp_average = 0;

  for (int i = 0; i < NUM_AVERAGE; i++) {
    /*
     * Classic malloc
     */
    start_time = (float)clock() / CLOCKS_PER_SEC;
    for (size_t i = 0; i < BENCHMARK_LENGTH; i++) {
      void *volatile buffer = malloc(ALLOCATION_SIZE_BYTES);
      free(buffer);
    }
    finish_time = (float)clock() / CLOCKS_PER_SEC;

    duration_ms = (finish_time - start_time) * 1000.0;
    malloc_average += duration_ms / (float)NUM_AVERAGE;

    /*
     * Memory pool
     */
    start_time = (float)clock() / CLOCKS_PER_SEC;
    memp_handle_t *pool_handle = memp_init(ALLOCATION_SIZE_BYTES, 5);
    for (size_t i = 0; i < BENCHMARK_LENGTH; i++) {
      void *volatile buffer = memp_malloc(pool_handle);
      memp_free(pool_handle, buffer);
    }
    finish_time = (float)clock() / CLOCKS_PER_SEC;

    duration_ms = (finish_time - start_time) * 1000.0;
    memp_average += duration_ms / (float)NUM_AVERAGE;
  }

  printf("%u allocations of %d bytes - %d runs average - malloc: %f ms\n",
         BENCHMARK_LENGTH, ALLOCATION_SIZE_BYTES, NUM_AVERAGE, malloc_average);
  printf("%u allocations of %d bytes - %d runs average - memp_malloc: %f ms\n",
         BENCHMARK_LENGTH, ALLOCATION_SIZE_BYTES, NUM_AVERAGE, memp_average);

  return 0;
}
