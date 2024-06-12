#include "allocators/pool_allocator.h"

#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

void test_memp_malloc_free(void) {
  const size_t block_size = 512;
  const size_t num_blocks = 10;

  memp_handle_t pool_handle;
  int result = memp_create(block_size, num_blocks, &pool_handle);
  TEST_ASSERT_EQUAL_INT(MEMP_PASS, result);
  TEST_ASSERT_NOT_NULL(pool_handle);
  TEST_ASSERT_EQUAL_INT(10, memp_stats_free_blocks(pool_handle));

  /*
   * Full memory pool
   */
  void *pools[num_blocks];
  for (int i = 0; i < num_blocks; i++) {
    void *temp_pool = memp_malloc(pool_handle);
    TEST_ASSERT_NOT_NULL(temp_pool);
    pools[i] = temp_pool;
  }

  /*
   * Empty memory pool
   */
  TEST_ASSERT_EQUAL_INT(0, memp_stats_free_blocks(pool_handle));
  void *temp_pool = memp_malloc(pool_handle);
  TEST_ASSERT_NULL(temp_pool);

  /*
   * Free memory
   */
  TEST_ASSERT_EQUAL_INT(0, memp_stats_free_blocks(pool_handle));
  for (int i = 0; i < num_blocks; i++) {
    memp_free(pool_handle, pools[i]);
  }
  TEST_ASSERT_EQUAL_INT(num_blocks, memp_stats_free_blocks(pool_handle));

  memp_destroy(pool_handle);
}

void test_memp_static_malloc_free(void) {
  const size_t block_size = 512;
  const size_t num_blocks = 10;

  const size_t buffer_size = (block_size + sizeof(void *)) * num_blocks + 48;
  uint8_t buffer[buffer_size];

  memp_handle_t pool_handle =
      memp_create_static(block_size, buffer, buffer_size);
  TEST_ASSERT_NOT_NULL(pool_handle);
  TEST_ASSERT_EQUAL_INT(10, memp_stats_free_blocks(pool_handle));

  /*
   * Full memory pool
   */
  void *pools[num_blocks];
  for (int i = 0; i < num_blocks; i++) {
    void *temp_pool = memp_malloc(pool_handle);
    TEST_ASSERT_NOT_NULL(temp_pool);
    pools[i] = temp_pool;
  }

  /*
   * Empty memory pool
   */
  TEST_ASSERT_EQUAL_INT(memp_stats_free_blocks(pool_handle), 0);
  void *temp_pool = memp_malloc(pool_handle);
  TEST_ASSERT_NULL(temp_pool);

  /*
   * Free memory
   */
  TEST_ASSERT_EQUAL_INT(memp_stats_free_blocks(pool_handle), 0);
  for (int i = 0; i < num_blocks; i++) {
    memp_free(pool_handle, pools[i]);
  }
  TEST_ASSERT_EQUAL_INT(memp_stats_free_blocks(pool_handle), num_blocks);
}

void test_memp_invalid_malloc_free(void) {
  const size_t block_size = 512;
  const size_t num_blocks = 10;

  /*
   * Memory initialisation
   */
  memp_handle_t pool_handle = NULL;
  int result;

  result = memp_create(0, 0, &pool_handle);
  TEST_ASSERT_EQUAL_INT(MEMP_INVALID_ARGS, result);
  TEST_ASSERT_NULL(pool_handle);

  result = memp_create(block_size, 0, &pool_handle);
  TEST_ASSERT_EQUAL_INT(MEMP_INVALID_ARGS, result);
  TEST_ASSERT_NULL(pool_handle);

  result = memp_create(0, num_blocks, &pool_handle);
  TEST_ASSERT_EQUAL_INT(MEMP_INVALID_ARGS, result);
  TEST_ASSERT_NULL(pool_handle);

  result = memp_create(block_size, num_blocks, &pool_handle);
  TEST_ASSERT_EQUAL_INT(MEMP_PASS, result);
  TEST_ASSERT_NOT_NULL(pool_handle);

  /*
   * Memory allocation
   */
  void *buffer;

  buffer = memp_malloc(NULL);
  TEST_ASSERT_NULL(buffer);
  TEST_ASSERT_EQUAL_INT(num_blocks, memp_stats_free_blocks(pool_handle));

  buffer = memp_malloc(pool_handle);
  TEST_ASSERT_NOT_NULL(buffer);
  TEST_ASSERT_EQUAL_INT(num_blocks - 1, memp_stats_free_blocks(pool_handle));

  memp_free(NULL, NULL);
  memp_free(pool_handle, NULL);
  memp_free(NULL, buffer);
  TEST_ASSERT_EQUAL_INT(num_blocks - 1, memp_stats_free_blocks(pool_handle));

  memp_free(pool_handle, buffer);
  TEST_ASSERT_EQUAL_INT(num_blocks, memp_stats_free_blocks(pool_handle));

  /*
   * Memory pool cleanup
   */
  memp_destroy(NULL);
  memp_destroy(pool_handle);
}

void test_memp_static_invalid_malloc_free(void) {
  const size_t block_size = 512;
  const size_t num_blocks = 10;

  const size_t buffer_size = (block_size + sizeof(void *)) * num_blocks + 32;
  uint8_t memp_buffer[buffer_size];
  const size_t small_buffer_size = 500;
  uint8_t small_memp_buffer[small_buffer_size];

  /*
   * Memory initialisation
   */
  memp_handle_t pool_handle = NULL;

  pool_handle = memp_create_static(0, NULL, 0);
  TEST_ASSERT_NULL(pool_handle);

  pool_handle = memp_create_static(0, memp_buffer, buffer_size);
  TEST_ASSERT_NULL(pool_handle);

  pool_handle = memp_create_static(block_size, NULL, buffer_size);
  TEST_ASSERT_NULL(pool_handle);

  pool_handle =
      memp_create_static(block_size, small_memp_buffer, small_buffer_size);
  TEST_ASSERT_NULL(pool_handle);

  pool_handle = memp_create_static(block_size, memp_buffer, buffer_size);
  TEST_ASSERT_NOT_NULL(pool_handle);

  /*
   * Memory allocation
   */
  void *buffer;

  buffer = memp_malloc(NULL);
  TEST_ASSERT_NULL(buffer);
  TEST_ASSERT_EQUAL_INT(num_blocks, memp_stats_free_blocks(pool_handle));

  buffer = memp_malloc(pool_handle);
  TEST_ASSERT_NOT_NULL(buffer);
  TEST_ASSERT_EQUAL_INT(num_blocks - 1, memp_stats_free_blocks(pool_handle));

  memp_free(NULL, NULL);
  memp_free(pool_handle, NULL);
  memp_free(NULL, buffer);
  TEST_ASSERT_EQUAL_INT(num_blocks - 1, memp_stats_free_blocks(pool_handle));

  memp_free(pool_handle, buffer);
  TEST_ASSERT_EQUAL_INT(num_blocks, memp_stats_free_blocks(pool_handle));

  /*
   * Memory pool cleanup
   */
  memp_destroy(NULL);
  memp_destroy(pool_handle);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_memp_malloc_free);
  RUN_TEST(test_memp_static_malloc_free);
  RUN_TEST(test_memp_invalid_malloc_free);
  RUN_TEST(test_memp_static_invalid_malloc_free);
  return UNITY_END();
}
