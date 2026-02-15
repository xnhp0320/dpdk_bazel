load("@rules_cc//cc:cc_binary.bzl", "cc_binary")


cc_binary(
    name = "hello_world",
    srcs = ["hello_world.cc"],
    deps = ["@abseil-cpp//absl/strings",
            "@dpdk//:meson_dpdk_libs"],
    linkopts = ["-lnuma", "-latomic"],
)

