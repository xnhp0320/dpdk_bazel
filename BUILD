load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@rules_proto//proto:defs.bzl", "proto_library")
load("@protobuf//bazel:cc_proto_library.bzl", "cc_proto_library")

# Proto library for messages
proto_library(
    name = "messages_proto",
    srcs = ["messages.proto"],
)

cc_proto_library(
    name = "messages_cc_proto",
    deps = [":messages_proto"],
)

# Unix socket server
cc_binary(
    name = "unix_socket_server",
    srcs = ["unix_socket_server.cc"],
    deps = [
        ":messages_cc_proto",
        "@boost.asio",
        "@protobuf//:protobuf",
    ],
)

# Unix socket client
cc_binary(
    name = "unix_socket_client",
    srcs = ["unix_socket_client.cc"],
    deps = [
        ":messages_cc_proto",
        "@boost.asio",
        "@protobuf//:protobuf",
    ],
)

# Original hello_world binary
cc_binary(
    name = "hello_world",
    srcs = ["hello_world.cc"],
    deps = [
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/flags:flag",
        "@abseil-cpp//absl/flags:parse",
        "@boost.asio",
        "@dpdk//:meson_dpdk_libs",
    ],
    linkopts = ["-lnuma", "-latomic"],
)

