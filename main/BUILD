load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "libs",
    srcs = glob(
        ["*.cc"],
        exclude = ["main.cc"],
    ),
    hdrs = glob(["*.hpp"]),
)

cc_binary(
    name = "cpplox",
    srcs = ["main.cc"],
    deps = [
        "libs",
    ],
)
