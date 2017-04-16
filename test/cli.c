#include "uv.h"
#include "uv_proxy_t.h"

#include "test/common.h"

static uv_link_t left;
static uv_link_t right;

static void test_reset() {
  counter = 0;

  free(left.data);
  free(right.data);
  left.data = NULL;
  right.data = NULL;
}


int main() {
  int err;

  uv_proxy_t p;
  uv_buf_t buf;

  test_reset();

  err = uv_link_init(&left, &uv_test_methods);
  CHECK_EQ(err, 0, "uv_link_init(left)");

  err = uv_link_init(&right, &uv_test_methods);
  CHECK_EQ(err, 0, "uv_link_init(right)");

  err = uv_proxy_init(&p);
  CHECK_EQ(err, 0, "uv_proxy_init(p)");

  err = uv_link_chain(&left, &p.left);
  CHECK_EQ(err, 0, "uv_link_chain(left)");

  err = uv_link_chain(&right, &p.right);
  CHECK_EQ(err, 0, "uv_link_chain(right)");

  /* PLAIN WRITE */

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

  /* EOF */

  uv_link_propagate_alloc_cb(&left, 1, &buf);
  uv_link_propagate_read_cb(&left, UV_EOF, &buf);

  CHECK_EQ(left.data, NULL, "EOF [l]");
  CHECK_EQ(strcmp(right.data, "[0] shutdown()\n"), 0, "EOF [r]");

  test_reset();

  return 0;
}
