#include "uv.h"
#include "uv_proxy_t.h"

#include "test/common.h"

int main() {
  int err;

  uv_link_t left;
  uv_link_t right;
  uv_proxy_t p;
  uv_buf_t buf;

  left.data = NULL;
  right.data = NULL;

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

  uv_link_propagate_alloc_cb(&left, 10, &buf);
  CHECK_NE(buf.base, NULL, "uv_link_propagate_alloc_cb");
  CHECK(buf.len >= 10, "uv_link_propagate_alloc_cb");

  memcpy(buf.base, "hello world", 10);
  uv_link_propagate_read_cb(&left, 32, &buf);

  fprintf(stderr, "%s", right.data);

  return 0;
}
