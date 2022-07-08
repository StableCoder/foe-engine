// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/filesystem.hpp>

#include <string_view>

TEST_CASE("std::filesystem - Stem of a path is correct") {
    REQUIRE(std::filesystem::path("/foo/bar.txt").stem() == "bar");
    REQUIRE(std::filesystem::path("foo/bar.txt").stem() == "bar");
}

TEST_CASE("foeGetUserHomeDirectory - Not empty") {
    auto test = foeGetUserHomeDirectory();

    REQUIRE(!test.empty());
}

#ifdef _WIN32
TEST_CASE("foeGetUserHomeDirectory - Home directory (Windows only)") {
    auto test = foeGetUserHomeDirectory();
    auto str = test.string();

    REQUIRE(str.find("Users") != std::string::npos);
}
#endif