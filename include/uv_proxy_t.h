#ifndef SRC_UV_PROXY_T_H_
#define SRC_UV_PROXY_T_H_

#include "uv_link_t.h"

typedef struct uv_proxy_s uv_proxy_t;

typedef void (*uv_proxy_error_cb)(uv_proxy_t* proxy, int err);
typedef void (*uv_proxy_close_cb)(uv_proxy_t* proxy);

struct uv_proxy_s {
  uv_link_t left;
  uv_link_t right;
  void* data;

  /* Internal fields */
  unsigned int shutdown:2;
  uv_proxy_error_cb error_cb;
  uv_proxy_close_cb close_cb;
};

int uv_proxy_init(uv_proxy_t* proxy, uv_proxy_error_cb cb);
void uv_proxy_close(uv_proxy_t* proxy, uv_proxy_close_cb cb);

#endif  /* SRC_UV_PROXY_T_H_ */
