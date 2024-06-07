#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * TODO Add interface for thread safety (mutex operations handle)
 */

/**
 * @brief Handle for initialised memory pools
 */
typedef struct memp_handle memp_handle_t;

/**
 * @brief Allocates the specified amount of memory for the memory pool
 * @param block_size The size of the blocks in the memory pool
 * @param num_blocks The number of blocks the pool should contain
 * @return A pointer to the initialised pool memory handle
 */
memp_handle_t *memp_init(size_t block_size, size_t num_blocks);

/**
 * @brief Free memory allocated previously using alloc_pool_memory
 * @param handle A pointer to the memory pool handle
 */
void memp_destroy(memp_handle_t *handle);

/**
 * @brief Allocate block from memory pool
 * @param handle A pointer to the memory pool handle
 */
void *memp_malloc(memp_handle_t *handle);

/**
 * @brief Free allocated block and add back to memory pool
 * @param handle A pointer to the memory pool handle
 * @param block A pointer to the previously allocated memory block
 */
void memp_free(memp_handle_t *handle, void *block);

/**
 * @brief Returns number of free memory blocks in memory pool
 */
int32_t memp_stats_free_blocks(memp_handle_t *handle);
