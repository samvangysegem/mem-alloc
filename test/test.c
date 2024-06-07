#include "allocators/pool_allocator.h"

#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

void test_memp_malloc_free(void) {
  const size_t buffer_size = 512;
  const size_t pool_size = 10;

  memp_handle_t *pool_handle = memp_init(buffer_size, pool_size);
  TEST_ASSERT_NOT_NULL(pool_handle);

  void *pools[pool_size];
  for (int i = 0; i < pool_size + 1; i++) {
    void *temp_pool = memp_malloc(pool_handle);
    if (i == pool_size) {
      TEST_ASSERT_NULL(temp_pool);
    } else {
      TEST_ASSERT_NOT_NULL(temp_pool);
      pools[i] = temp_pool;
    }
  }

  for (int i = 0; i < pool_size; i++) {
    memp_free(pool_handle, pools[i]);
  }

  memp_destroy(pool_handle);
}

void test_memp_invalid_malloc_free(void) {
  const size_t buffer_size = 512;
  const size_t pool_size = 10;

  /*
   * Memory initialisation
   */
  memp_handle_t *pool_handle = NULL;

  pool_handle = memp_init(0, 0);
  TEST_ASSERT_NULL(pool_handle);

  pool_handle = memp_init(buffer_size, 0);
  TEST_ASSERT_NULL(pool_handle);

  pool_handle = memp_init(0, pool_size);
  TEST_ASSERT_NULL(pool_handle);

  pool_handle = memp_init(buffer_size, pool_size);
  TEST_ASSERT_NOT_NULL(pool_handle);
  TEST_ASSERT_TRUE(memp_stats_free_blocks(pool_handle) == pool_size);

  /*
   * Memory allocation
   */
  void *buffer;

  buffer = memp_malloc(NULL);
  TEST_ASSERT_NULL(buffer);
  TEST_ASSERT_TRUE(memp_stats_free_blocks(pool_handle) == pool_size);

  buffer = memp_malloc(pool_handle);
  TEST_ASSERT_NOT_NULL(buffer);
  TEST_ASSERT_TRUE(memp_stats_free_blocks(pool_handle) == pool_size - 1);

  memp_free(NULL, NULL);
  memp_free(pool_handle, NULL);
  memp_free(NULL, buffer);
  TEST_ASSERT_TRUE(memp_stats_free_blocks(pool_handle) == pool_size - 1);

  memp_free(pool_handle, buffer);
  TEST_ASSERT_TRUE(memp_stats_free_blocks(pool_handle) == pool_size);

  /*
   * Memory pool cleanup
   */
  memp_destroy(NULL);
  memp_destroy(pool_handle);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_memp_malloc_free);
  RUN_TEST(test_memp_invalid_malloc_free);
  return UNITY_END();
}
