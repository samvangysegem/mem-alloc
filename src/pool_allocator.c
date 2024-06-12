#include "allocators/pool_allocator.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORD_SIZE sizeof(void *)

/*
 * Flags
 */
#define MEMP_FLAGS_DYNAMIC_ALLOC_OFFSET 0
#define MEMP_FLAGS_DYNAMIC_ALLOC_MASK (1 << 0)

/*
 * Memory block handle
 * Includes the size as header
 */
typedef struct memp_block {
  /* Header */
  uint32_t size;

  /* Data */
  struct memp_block *next;
} memp_block_t;

typedef struct memp_pool {
#ifdef MEMP_STATS
  int32_t free_blocks;
  int32_t used_blocks;
  int32_t max_used_blocks;
#endif

  /* Blocks details */
  memp_block_t *pool_p;
  uint32_t block_size;
  uint32_t flags;
} memp_pool_t;

/*
 * Stack linked list helper functions
 */

#define node_t memp_block_t

static void push(node_t **base, node_t *node) {
  node->next = *base;
  *base = node;
}

static node_t *pop(node_t **base) {
  if (!*base) {
    return NULL;
  }

  node_t *result = *base;
  *base = (*base)->next;
  return result;
}

/*
 * Helper functions
 */

static inline int32_t max(int32_t a, int32_t b) { return (a > b) ? a : b; }

/**
 * @brief Aligns provided size with word size
 */
static inline size_t align(const size_t size) {
  return (size + WORD_SIZE - 1) & ~(WORD_SIZE - 1);
}

/**
 * @brief Returns the size of a memory aligned block
 */
static inline size_t aligned_block_size(const size_t block_size) {
  return align(block_size);
}

/**
 * @brief Returns the size of a memory aligned block with its header
 */
static inline size_t aligned_block_with_header_size(const size_t block_size) {
  return aligned_block_size(block_size) + sizeof(memp_block_t) -
         sizeof(memp_block_t *);
}

/**
 * @brief Returns the size of a memory pool consisting of the provided number of
 * blocks, after being aligned and adding their respective headers
 */
static inline size_t aligned_pool_size(const size_t block_size,
                                       const size_t num_blocks) {
  return num_blocks * aligned_block_with_header_size(block_size);
}

/**
 * @brief Initialises the provided buffer as a memory pool, with the first bytes
 * of the buffer serving as memory pool handle and the remaining bytes serving
 * as memory pool blocks. This function expects valid arguments and does not
 * perform any additional validation of these arguments!
 */
static memp_handle_t memp_init_buffer(const size_t block_size,
                                      const size_t num_blocks,
                                      uint8_t *const buffer) {
  /* Initialise memory pool handle */
  memp_pool_t *memp_handle = (memp_pool_t *)buffer;

  /* Initialise stack linked list of memory blocks in remaining buffer */
  size_t block_address = (size_t)memp_handle + sizeof(memp_pool_t);
  for (uint32_t i = 0; i < num_blocks; i++) {
    memp_block_t *block = (memp_block_t *)block_address;
    block->size = aligned_block_size(block_size);
    push(&memp_handle->pool_p, block);

    /* Update address to next block */
    block_address += aligned_block_with_header_size(block_size);
  }

  /* Should match the block header entry */
  memp_handle->block_size = aligned_block_size(block_size);
#ifdef MEMP_STATS
  memp_handle->free_blocks = num_blocks;
  memp_handle->used_blocks = 0;
  memp_handle->max_used_blocks = 0;
#endif

  return memp_handle;
}

/*
 * Memory management functions
 */

memp_result_t memp_create(const size_t block_size, const size_t num_blocks,
                          memp_handle_t *const handle) {
  if (block_size == 0 || num_blocks == 0) {
    return MEMP_INVALID_ARGS;
  }

  /* Allocate memory for memory pool handle and memory pool */
  uint8_t *buffer = (uint8_t *)malloc(
      sizeof(memp_pool_t) + aligned_pool_size(block_size, num_blocks));
  if (!buffer)
    return MEMP_ALLOC_FAILED;

  /* Initialise allocated buffer */
  memset(buffer, 0x00,
         sizeof(memp_pool_t) + aligned_pool_size(block_size, num_blocks));

  /* Update handle */
  *handle = memp_init_buffer(block_size, num_blocks, buffer);
  (*handle)->flags |= (1 << MEMP_FLAGS_DYNAMIC_ALLOC_OFFSET);

  return MEMP_PASS;
}

memp_handle_t memp_create_static(const size_t block_size, uint8_t *const buffer,
                                 const size_t buffer_size) {
  if (block_size == 0 || !buffer || buffer_size == 0) {
#ifdef MEMP_DEBUG
    puts("Invalid arguments passed to 'memp_create_static");
#endif
    return NULL;
  }

  if (buffer_size < aligned_pool_size(block_size, 1) + sizeof(memp_pool_t)) {
#ifdef MEMP_DEBUG
    puts("Buffer passed to 'memp_create_static' is not large enough");
#endif
    return NULL;
  }

  /* Initialise buffer */
  memset(buffer, 0x00, buffer_size);

  /* Calculate number of blocks that fit in the buffer */
  size_t num_blocks = (buffer_size - sizeof(memp_pool_t)) /
                      aligned_block_with_header_size(block_size);

  return memp_init_buffer(block_size, num_blocks, buffer);
}

void memp_destroy(memp_handle_t const handle) {
  if (!handle) {
#ifdef MEMP_DEBUG
    puts("Invalid handle passed to 'memp_destroy'");
#endif
    return;
  }

  if (handle->flags & MEMP_FLAGS_DYNAMIC_ALLOC_MASK) {
    free(handle);
  } else {
#ifdef MEMP_DEBUG
    puts("Attempted to call `memp_destroy` on statically allocated buffer!");
#endif /* ifdef MEMP_DEBUG */
  }
}

void *memp_malloc(memp_handle_t const handle) {
  if (!handle) {
#ifdef MEMP_DEBUG
    puts("Invalid handle passed to 'memp_malloc'");
#endif
    return NULL;
  }

  if (handle->pool_p == NULL) {
#ifdef MEMP_DEBUG
    puts("Memory pool is empty");
#endif
    return NULL;
  }

  memp_block_t *block = pop(&handle->pool_p);
#ifdef MEMP_STATS
  handle->free_blocks--;
  handle->used_blocks++;
  handle->max_used_blocks = max(handle->max_used_blocks, handle->used_blocks);
#endif

  /* Remove pointer to next block at the start of the allocated memory  */
  block->next = NULL;
  return (void *)(&block->next);
}

void memp_free(memp_handle_t const handle, void *const block) {
  if (!handle || !block) {
#ifdef MEMP_DEBUG
    puts("Invalid handle passed to 'memp_free'");
#endif
    return;
  }

  /* Get header */
  size_t offset = offsetof(memp_block_t, next);
  memp_block_t *block_handle = (memp_block_t *)((char *)block - offset);

  /* Validate block is part of the pool */
  if (block_handle->size != handle->block_size) {
#ifdef MEMP_DEBUG
    puts("Invalid block handle passed to 'memp_free': incorrect block size");
#endif
    return;
  }

#ifdef MEMP_STATS
  handle->free_blocks++;
  handle->used_blocks--;
#endif

  push(&handle->pool_p, block_handle);
}

int32_t memp_stats_free_blocks(memp_handle_t const handle) {
#ifdef MEMP_STATS
  return handle->free_blocks;
#else
  return -1;
#endif
}

int32_t memp_stats_used_blocks(memp_handle_t const handle) {
#ifdef MEMP_STATS
  return handle->used_blocks;
#else
  return -1;
#endif
}

int32_t memp_stats_max_used_blocks(memp_handle_t const handle) {
#ifdef MEMP_STATS
  return handle->max_used_blocks;
#else
  return -1;
#endif
}
