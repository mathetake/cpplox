load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "tests",
    srcs = glob(["**/*.cc"]),
    deps = [
        "//main:libs",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "compiler",
    srcs = ["compiler_test.cc"],
    deps = [
        "//main:libs",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "table",
    srcs = ["table_test.cc"],
    deps = [
        "//main:libs",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "object",
    srcs = ["object_test.cc"],
    deps = [
        "//main:libs",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "chunk",
    srcs = ["chunk_test.cc"],
    deps = [
        "//main:libs",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "value",
    srcs = ["value_test.cc"],
    deps = [
        "//main:libs",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "vm",
    srcs = ["vm_test.cc"],
    deps = [
        "//main:libs",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "scanner",
    srcs = ["scanner_test.cc"],
    deps = [
        "//main:libs",
        "@googletest//:gtest_main",
    ],
)
