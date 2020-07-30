load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "tests",
    srcs = glob(["**/*.cc"]),
    deps = [
        "//main:chunk",
        "//main:compiler",
        "//main:scanner",
        "//main:value",
        "//main:vm",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "compiler",
    srcs = ["compiler_test.cc"],
    deps = [
        "//main:compiler",
        "//main:value",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "chunk",
    srcs = ["chunk_test.cc"],
    deps = [
        "//main:chunk",
        "//main:value",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "value",
    srcs = ["value_test.cc"],
    deps = [
        "//main:value",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "vm",
    srcs = ["vm_test.cc"],
    deps = [
        "//main:value",
        "//main:vm",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "scanner",
    srcs = ["scanner_test.cc"],
    deps = [
        "//main:scanner",
        "@googletest//:gtest_main",
    ],
)
