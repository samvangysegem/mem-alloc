#include "allocators/pool_allocator.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORD_SIZE sizeof(void *)

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

typedef struct memp_handle {
#ifdef MEMP_STATS
  int32_t free_blocks;
  int32_t used_blocks;
  int32_t max_used_blocks;
#endif

  /* Blocks details */
  uint32_t block_size;
  memp_block_t *pool_p;
} memp_handle_t;

/*
 * Stack Linked List helper functions
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
static inline size_t align(size_t size) {
  return (size + WORD_SIZE - 1) & ~(WORD_SIZE - 1);
}

static inline int32_t max(int32_t a, int32_t b) { return (a > b) ? a : b; }

/*
 * Memory management functions
 */
memp_handle_t *memp_init(size_t const block_size, size_t const num_blocks) {
  if (block_size == 0 || num_blocks == 0) {
    return NULL;
  }

  /*
   * Allocate memory for memory pool handle and memory pool
   */
  size_t const aligned_block_size = align(block_size);
  size_t const total_block_size =
      aligned_block_size + sizeof(memp_block_t) - sizeof(memp_block_t *);
  size_t const pool_size = num_blocks * total_block_size;
  memp_handle_t *memp_handle =
      (memp_handle_t *)malloc(sizeof(memp_handle_t) + pool_size);
  if (!memp_handle) {
    return NULL;
  }

  /*
   * Initialise allocated memory to zero
   */
  memset(memp_handle, 0x00, sizeof(memp_handle_t) + pool_size);

  /*
   * Initialise stack linked list of memory blocks in allocated memory
   */
  size_t block_address = (size_t)memp_handle + sizeof(memp_handle_t);
  for (uint32_t i = 0; i < num_blocks; i++) {
    memp_block_t *block = (memp_block_t *)block_address;
    block->size = aligned_block_size;
    push(&memp_handle->pool_p, block);

    /* Update address to next block */
    block_address += total_block_size;
  }

  /* Should match the block header entry */
  memp_handle->block_size = aligned_block_size;
#ifdef MEMP_STATS
  memp_handle->free_blocks = num_blocks;
  memp_handle->used_blocks = 0;
  memp_handle->max_used_blocks = 0;
#endif

  return memp_handle;
}

void memp_destroy(memp_handle_t *handle) {
  if (!handle) {
#ifdef MEMP_DEBUG
    puts("Invalid handle passed to 'memp_destroy'");
#endif
    return;
  }

  free(handle);
}

void *memp_malloc(memp_handle_t *handle) {
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

void memp_free(memp_handle_t *handle, void *block) {
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

int32_t memp_stats_free_blocks(memp_handle_t *handle) {
#ifdef MEMP_STATS
  return handle->free_blocks;
#else
  return -1;
#endif
}
