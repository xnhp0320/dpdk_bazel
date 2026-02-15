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

Notes:
1. why pmdinfogen.py patches?
because the rule_foreign_cc is calling python3 using relative path:
python3 xxx/yyy/zzz/../../pmdinfogen.py ....

In this case the python3 interpreter fails to `import coff`, as coff.py is located
in the same places where pmdinfogen.py is at.

Honestly speaking this is a bug of DPDK pmdinfogen.py.
It's hard to blame bazel.
