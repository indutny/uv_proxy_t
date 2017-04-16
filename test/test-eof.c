#include "test/common.h"

TEST_IMPL(eof) {
  test_init();

  uv_link_propagate_alloc_cb(&left, 1, &buf);
  uv_link_propagate_read_cb(&left, UV_EOF, &buf);

  CHECK_EQ(left.data, NULL, "EOF [l]");
  CHECK_EQ(strcmp(right.data, "[0] shutdown()\n"), 0, "EOF [r]");

  test_reset();
}
