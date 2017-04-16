#include "test/common.h"

TEST_IMPL(async_write) {
  test_init();

  uv_link_propagate_alloc_cb(&left, 11, &buf);
  CHECK_NE(buf.base, NULL, "uv_link_propagate_alloc_cb");
  CHECK(buf.len >= 11, "uv_link_propagate_alloc_cb");

  memcpy(buf.base, "hello world", 11);
  uv_link_propagate_read_cb(&left, 11, &buf);

  CHECK_EQ(strcmp(left.data, "[1] read_stop()\n"
                             "[3] read_start()\n"), 0,
           "plain write [l]");
  CHECK_EQ(strcmp(right.data, "[0] try_write(1): 11:\"hello world\"\n"
                              "[2] write(1): 11:\"hello world\"\n"), 0,
           "plain write [r]");

  test_reset();
  /* FULL INSTANT WRITE */

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

  /* PARTIAL WRITE */

  partial_try_write = 5;

  uv_link_propagate_alloc_cb(&left, 11, &buf);
  CHECK_NE(buf.base, NULL, "uv_link_propagate_alloc_cb");
  CHECK(buf.len >= 11, "uv_link_propagate_alloc_cb");

  memcpy(buf.base, "hello world", 11);
  uv_link_propagate_read_cb(&left, 11, &buf);

  CHECK_EQ(strcmp(left.data, "[1] read_stop()\n"
                             "[3] read_start()\n"), 0,
           "partial write [l]");
  CHECK_EQ(strcmp(right.data, "[0] try_write(1): 11:\"hello world\"\n"
                              "[2] write(1): 6:\" world\"\n"), 0,
           "partial write [r]");

  test_reset();

  /* EOF */

  uv_link_propagate_alloc_cb(&left, 1, &buf);
  uv_link_propagate_read_cb(&left, UV_EOF, &buf);

  CHECK_EQ(left.data, NULL, "EOF [l]");
  CHECK_EQ(strcmp(right.data, "[0] shutdown()\n"), 0, "EOF [r]");

  test_reset();
}
