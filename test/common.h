#ifndef TEST_COMMON_H_
#define TEST_COMMON_H_

#include <stdlib.h>
#include <string.h>

#define CHECK(VALUE, MESSAGE)                                                \
    do {                                                                     \
      if ((VALUE)) break;                                                    \
      fprintf(stderr, "Assertion failure: " #MESSAGE "\n");                  \
      abort();                                                               \
    } while (0)

#define CHECK_EQ(A, B, MESSAGE) CHECK((A) == (B), MESSAGE)
#define CHECK_NE(A, B, MESSAGE) CHECK((A) != (B), MESSAGE)


static unsigned int counter;


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

  uv_test_log(link, "[%d] try_write(%u):", counter++, nbufs);
  for (i = 0; i < nbufs; i++) {
    uv_test_log(link, " %d:\"%.*s\"", (int) bufs[i].len, bufs[i].len,
                bufs[i].base);
  }
  uv_test_log(link, "\n");

  return 0;
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

#endif  /* TEST_COMMON_H_ */
