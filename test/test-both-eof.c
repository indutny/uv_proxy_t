#include "test/common.h"

static unsigned int close_cb_called;

static void both_eof_close_cb(uv_proxy_t* p) {
  close_cb_called++;
}

static void both_eof_error_cb(uv_proxy_t* p, uv_link_t* link, int err) {
  CHECK_EQ(link, NULL, "EOF error shouldn't have link");
  CHECK_EQ(err, UV_EOF, "error_cb() must be invoked with UV_EOF");

  uv_test_log(&left, "GLOBAL EOF\n");
  uv_test_log(&right, "GLOBAL EOF\n");

  uv_proxy_close(p, both_eof_close_cb);
}


TEST_IMPL(both_eof) {
  test_init();

  p.error_cb = both_eof_error_cb;

  uv_link_propagate_alloc_cb(&left, 1, &buf);
  uv_link_propagate_read_cb(&left, UV_EOF, &buf);

  uv_link_propagate_alloc_cb(&right, 1, &buf);
  uv_link_propagate_read_cb(&right, UV_EOF, &buf);

  CHECK_EQ(close_cb_called, 1, "close_cb must be called");
  CHECK_EQ(strcmp(left.data, "[1] shutdown()\n"
                             "GLOBAL EOF\n"
                             "[3] close()\n"), 0,
           "BOTH EOF [l]");
  CHECK_EQ(strcmp(right.data, "[0] shutdown()\n"
                              "GLOBAL EOF\n"
                              "[2] close()\n"), 0,
           "BOTH EOF [r]");

  test_reset();
}
