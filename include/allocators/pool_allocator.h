#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Error handling
 */

typedef enum {
  MEMP_PASS = 0,
  MEMP_FAIL = -1,
  MEMP_INVALID_ARGS = -10,
  MEMP_ALLOC_FAILED = -11
} memp_result_t;

/*
 * Memory pool
 */

typedef struct memp_pool memp_pool_t;
typedef memp_pool_t *memp_handle_t;

/**
 * @brief Allocates the specified amount of memory for the memory pool
 * @param block_size The size of the blocks in the memory pool
 * @param num_blocks The number of blocks the pool should contain
 * @param handle Used to pass the handle of the created memory pool handle out
 * of the memp_create function
 * @return A result indicating success (0) or the relevant error code
 */
memp_result_t memp_create(const size_t block_size, const size_t num_blocks,
                          memp_handle_t *const handle);

/**
 * @brief Initialises the provided buffer as pool memory. The user is
 * responsible for sizing the buffer correctly and is advised to adhere to the
 * following formula: buffer_size = (block_size + WORD_SIZE) * num_blocks + 50.
 * Not following this formula might result in a lower num_blocks value than
 * expected.
 * @param block_size The size of the blocks in the memory pool
 * @param buffer Must point to an array that has at least `buffer_size` indices
 * @param buffer_size Must be set to the number of indices in the `buffer` array
 * and should adhere to the formula mentioned above
 * @return If all arguments are valid and the buffer is of sufficient size, a
 * handle to the memory pool will be returned. In any other case, NULL will be
 * returned.
 */
memp_handle_t memp_create_static(const size_t block_size, uint8_t *const buffer,
                                 const size_t buffer_size);

/**
 * @brief Free memory allocated previously using alloc_pool_memory
 * @param handle A pointer to the memory pool handle
 */
void memp_destroy(memp_handle_t const handle);

/**
 * @brief Allocate block from memory pool
 * @param handle A pointer to the memory pool handle
 */
void *memp_malloc(memp_handle_t const handle);

/**
 * @brief Free allocated block and add back to memory pool
 * @param handle A pointer to the memory pool handle
 * @param block A pointer to the previously allocated memory block
 */
void memp_free(memp_handle_t const handle, void *const block);

/**
 * @brief Returns number of free memory blocks in memory pool
 */
int32_t memp_stats_free_blocks(memp_handle_t const handle);

/**
 * @brief Returns number of free memory blocks in memory pool
 */
int32_t memp_stats_used_blocks(memp_handle_t const handle);

/**
 * @brief Returns number of free memory blocks in memory pool
 */
int32_t memp_stats_max_used_blocks(memp_handle_t const handle);
