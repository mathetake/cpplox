load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "common",
    hdrs = ["common.hpp"],
)

cc_library(
    name = "value",
    srcs = ["value.cc"],
    hdrs = ["value.hpp"],
    deps = ["common"],
)

cc_library(
    name = "debug",
    srcs = ["debug.cc"],
    hdrs = ["debug.hpp"],
    deps = ["chunk"],
)

cc_library(
    name = "scanner",
    srcs = ["scanner.cc"],
    hdrs = ["scanner.hpp"],
    deps = [
        "common",
    ],
)

cc_library(
    name = "compiler",
    srcs = ["compiler.cc"],
    hdrs = ["compiler.hpp"],
    deps = [
        "chunk",
        "common",
        "debug",
        "scanner",
    ],
)

cc_library(
    name = "chunk",
    srcs = ["chunk.cc"],
    hdrs = ["chunk.hpp"],
    deps = [
        "value",
    ],
)

cc_library(
    name = "vm",
    srcs = ["vm.cc"],
    hdrs = ["vm.hpp"],
    deps = [
        "chunk",
        "common",
        "compiler",
        "debug",
    ],
)

cc_binary(
    name = "cpplox",
    srcs = ["main.cc"],
    deps = [
        "chunk",
        "common",
        "debug",
        "vm",
    ],
)
