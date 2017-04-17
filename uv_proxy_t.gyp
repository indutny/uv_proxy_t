{
  "targets": [ {
    "target_name": "uv_proxy_t",
    "type": "<!(gypkg type)",

    "variables": {
      "gypkg_deps": [
        "git://github.com/indutny/uv_link_t@^1.0.0 [gpg] => uv_link_t.gyp:uv_link_t",
        "git://github.com/libuv/libuv.git@^1.9.0 => uv.gyp:libuv",
      ],
    },

    "dependencies": [
      "<!@(gypkg deps <(gypkg_deps))",
    ],

    "direct_dependent_settings": {
      "include_dirs": [
        "include",
      ],
    },

    "include_dirs": [
      ".",
    ],

    "sources": [
      "src/proxy.c",
    ],
  }, {
    "target_name": "uv_proxy_t-test",
    "type": "executable",

    "variables": {
      "gypkg_deps": [
        "git://github.com/indutny/uv_link_t@^1.0.0 [gpg] => uv_link_t.gyp:uv_link_t",
        "git://github.com/libuv/libuv.git@^1.9.0 => uv.gyp:libuv",
        "git://github.com/indutny/mini-test.c.git@^1.0.0 => mini-test.gyp:mini-test",
      ],
    },

    "dependencies": [
      "<!@(gypkg deps <(gypkg_deps))",
      "uv_proxy_t",
    ],

    "include_dirs": [
      ".",
    ],

    "sources": [
      "test/main.c",
      "test/test-async-write.c",
      "test/test-both-eof.c",
      "test/test-eof.c",
      "test/test-partial-write.c",
      "test/test-sync-write.c",
    ],
  } ],
}
