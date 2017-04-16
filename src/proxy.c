#include <stdlib.h>

#include "include/uv_proxy_t.h"

static void uv_proxy_link_close_cb(uv_link_t* link);
static void uv_proxy_alloc_cb(uv_link_t* link,
                              size_t suggested_size,
                              uv_buf_t* buf);
static void uv_proxy_read_cb(uv_link_t* link,
                             ssize_t nread,
                             const uv_buf_t* buf);
static void uv_proxy_shutdown_cb(uv_link_t* source, int status, void* arg);
static void uv_proxy_write_cb(uv_link_t* source, int status, void* arg);
static uv_link_t* uv_proxy_get_other(uv_proxy_t* proxy, uv_link_t* link);


static const size_t kProxyChunkLength = 16384;


static uv_link_methods_t uv_proxy_methods = {
  .read_start = uv_link_default_read_start,
  .read_stop = uv_link_default_read_stop,
  .write = uv_link_default_write,
  .try_write = uv_link_default_try_write,
  .shutdown = uv_link_default_shutdown,
  .close = uv_link_default_close,

  .alloc_cb_override = uv_proxy_alloc_cb,
  .read_cb_override = uv_proxy_read_cb
};


void uv_proxy_link_close_cb(uv_link_t* link) {
  uv_proxy_t* p;
  uv_proxy_close_cb cb;

  p = link->data;
  link->data = NULL;

  /* One more link pending */
  if (p->left.data != p->right.data)
    return;

  cb = p->close_cb;
  p->close_cb = NULL;
  if (cb != NULL)
    cb(p);
}


void uv_proxy_alloc_cb(uv_link_t* link, size_t suggested_size, uv_buf_t* buf) {
  size_t len;
  char* base;

  /* TODO(indutny): use ringbuffer */
  len = kProxyChunkLength;
  base = malloc(len);

  *buf = uv_buf_init(base, len);
}


void uv_proxy_read_cb(uv_link_t* link, ssize_t nread, const uv_buf_t* buf) {
  uv_proxy_t* p;
  uv_link_t* other;
  uv_buf_t buf_copy;
  int err;

  p = link->data;
  other = uv_proxy_get_other(p, link);

  if (nread < 0) {
    if (nread == UV_EOF) {
      free(buf->base);
      err = uv_link_shutdown(other, uv_proxy_shutdown_cb, NULL);

      /* TODO(indutny): handle error */
      if (err != 0)
        abort();
    }

    /* TODO(indutny): handle error */
    goto fatal;
  }

  buf_copy.base = buf->base;
  buf_copy.len = nread;
  err = uv_link_try_write(other, &buf_copy, 1);

  /* TODO(indutny): handle error */
  if (err < 0 && err != UV_EAGAIN && err != UV_ENOSYS)
    goto fatal;

  buf_copy.len -= err;
  buf_copy.base += err;

  /* Completely written */
  if (buf_copy.len == 0) {
    free(buf->base);
    return;
  }

  /* Asynchronous write needed */
  err = uv_link_read_stop(link);
  if (err != 0)
    goto fatal;

  err = uv_link_write(other, &buf_copy, 1, NULL, uv_proxy_write_cb, buf->base);

  /* TODO(indutny): handle error */
  if (err != 0)
    goto fatal;

  return;

fatal:
  free(buf->base);
  abort();
}


void uv_proxy_shutdown_cb(uv_link_t* source, int status, void* arg) {
  /* No-op */
}


void uv_proxy_write_cb(uv_link_t* source, int status, void* arg) {
  uv_proxy_t* p;
  int err;

  p = source->data;

  free(arg);
  if (status == UV_ECANCELED)
    return;

  /* TODO(indutny): report errors */
  if (status < 0) {
    abort();
    return;
  }

  err = uv_link_read_start(uv_proxy_get_other(p, source));

  /* TODO(indutny): report errors */
  if (err != 0) {
    abort();
    return;
  }
}


int uv_proxy_init(uv_proxy_t* proxy) {
  int err;

  proxy->left.data = NULL;
  proxy->right.data = NULL;
  proxy->close_cb = NULL;

  err = uv_link_init(&proxy->left, &uv_proxy_methods);
  if (err != 0)
    return err;

  proxy->left.data = proxy;

  err = uv_link_init(&proxy->right, &uv_proxy_methods);
  if (err != 0)
    goto fail;

  proxy->right.data = proxy;

  proxy->left.alloc_cb = uv_proxy_alloc_cb;
  proxy->left.read_cb = uv_proxy_read_cb;

  return err;

fail:
  uv_link_close(&proxy->left, uv_proxy_link_close_cb);
  return err;
}


void uv_proxy_close(uv_proxy_t* proxy, uv_proxy_close_cb cb) {
  proxy->close_cb = cb;
  uv_link_close(&proxy->left, uv_proxy_link_close_cb);
  uv_link_close(&proxy->right, uv_proxy_link_close_cb);
}


/* Auxiliary methods */


uv_link_t* uv_proxy_get_other(uv_proxy_t* proxy, uv_link_t* link) {
  if (&proxy->left == link)
    return &proxy->right;
  else
    return &proxy->left;
}
