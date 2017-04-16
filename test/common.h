#ifndef TEST_COMMON_H_
#define TEST_COMMON_H_

#include "uv.h"
#include "uv_proxy_t.h"

#include "test/test-list.h"
#include "mini/test.h"

static uv_link_t left;
static uv_link_t right;
static uv_proxy_t p;
static uv_buf_t buf;

static unsigned int counter;
static int partial_try_write;


static void uv_test_error_cb(uv_proxy_t* p, uv_link_t* side, int err) {
  abort();
}


static void uv_test_log(uv_link_t* link, char* fmt, ...) {
  va_list ap;
  va_list ap_copy;
  const char* left;
  char* buf;
  char* p;
  int len;

  va_start(ap, fmt);

  left = link->data == NULL ? "" : link->data;

  va_copy(ap_copy, ap);
  len = vsnprintf(NULL, 0, fmt, ap_copy);
  va_end(ap_copy);

  buf = malloc(strlen(left) + len + 1);
  CHECK_NE(buf, NULL, "malloc() in uv_test_log");

  p = buf;
  p += sprintf(p, "%s", left);
  p += vsprintf(p, fmt, ap);

  free(link->data);
  link->data = buf;

  va_end(ap);
}

static int uv_test_read_start(uv_link_t* link) {
  uv_test_log(link, "[%d] read_start()\n", counter++);
  return 0;
}


static int uv_test_read_stop(uv_link_t* link) {
  uv_test_log(link, "[%d] read_stop()\n", counter++);
  return 0;
}


static int uv_test_write(uv_link_t* link,
                         uv_link_t* source,
                         const uv_buf_t bufs[],
                         unsigned int nbufs,
                         uv_stream_t* send_handle,
                         uv_link_write_cb cb,
                         void* arg) {
  unsigned int i;

  uv_test_log(link, "[%d] write(%u):", counter++, nbufs);
  for (i = 0; i < nbufs; i++) {
    uv_test_log(link, " %d:\"%.*s\"", (int) bufs[i].len, bufs[i].len,
                bufs[i].base);
  }
  uv_test_log(link, "\n");

  cb(source, 0, arg);

  return 0;
}


static int uv_test_try_write(uv_link_t* link,
                             const uv_buf_t bufs[],
                             unsigned int nbufs) {
  unsigned int i;
  int r;

  uv_test_log(link, "[%d] try_write(%u):", counter++, nbufs);
  for (i = 0; i < nbufs; i++) {
    uv_test_log(link, " %d:\"%.*s\"", (int) bufs[i].len, bufs[i].len,
                bufs[i].base);
  }
  uv_test_log(link, "\n");

  r = partial_try_write;
  partial_try_write = 0;
  return r;
}


static int uv_test_shutdown(uv_link_t* link,
                            uv_link_t* source,
                            uv_link_shutdown_cb cb,
                            void* arg) {
  uv_test_log(link, "[%d] shutdown()\n", counter++);
  cb(source, 0, arg);

  return 0;
}


static void uv_test_close(uv_link_t* link, uv_link_t* source,
                          uv_link_close_cb cb) {
  uv_test_log(link, "[%d] close()\n", counter++);
  cb(source);
}


static uv_link_methods_t uv_test_methods = {
  .read_start = uv_test_read_start,
  .read_stop = uv_test_read_stop,
  .write = uv_test_write,
  .try_write = uv_test_try_write,
  .shutdown = uv_test_shutdown,
  .close = uv_test_close
};


static void test_init() {
  int err;

  err = uv_link_init(&left, &uv_test_methods);
  CHECK_EQ(err, 0, "uv_link_init(left)");

  err = uv_link_init(&right, &uv_test_methods);
  CHECK_EQ(err, 0, "uv_link_init(right)");

  err = uv_proxy_init(&p, uv_test_error_cb);
  CHECK_EQ(err, 0, "uv_proxy_init(p)");

  err = uv_link_chain(&left, &p.left);
  CHECK_EQ(err, 0, "uv_link_chain(left)");

  err = uv_link_chain(&right, &p.right);
  CHECK_EQ(err, 0, "uv_link_chain(right)");
}


static void test_reset() {
  counter = 0;

  free(left.data);
  free(right.data);
  left.data = NULL;
  right.data = NULL;
}

#endif  /* TEST_COMMON_H_ */
