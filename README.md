# dpdk_bazel
use bazel to build dpdk.

But you still have to do something before bazel build

1. install python3-pyelftools, this is required by meson build
2. install libnuma, for examples: apt install libnuma-dev.

you can:

```
bazel build @dpdk//:meson_dpdk_libs # build dpdk
bazel build //:hello_world # build dpdk and abseil-cpp.

```
