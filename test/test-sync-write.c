#include "test/common.h"

TEST_IMPL(sync_write) {
  test_init();

  partial_try_write = 11;

  uv_link_propagate_alloc_cb(&left, 11, &buf);
  CHECK_NE(buf.base, NULL, "uv_link_propagate_alloc_cb");
  CHECK(buf.len >= 11, "uv_link_propagate_alloc_cb");

  memcpy(buf.base, "hello world", 11);
  uv_link_propagate_read_cb(&left, 11, &buf);

  CHECK_EQ(left.data, NULL, "full instant write [l]");
  CHECK_EQ(strcmp(right.data, "[0] try_write(1): 11:\"hello world\"\n"), 0,
           "full instant write [r]");

  test_reset();
}
