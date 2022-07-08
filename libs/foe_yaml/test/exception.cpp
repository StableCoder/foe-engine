// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/yaml/exception.hpp>

TEST_CASE("foeYamlException - String constructor", "[foe][yaml]") {
    foeYamlException test{"test"};

    REQUIRE(std::string{test.what()} == "test");
    REQUIRE(test.whatStr() == "test");
}

TEST_CASE("foeYamlException - Copy constructor", "[foe][yaml]") {
    foeYamlException copy{"test"};
    foeYamlException test{copy};

    REQUIRE(std::string{test.what()} == "test");
    REQUIRE(test.whatStr() == "test");
}

TEST_CASE("foeYamlException - Move constructor", "[foe][yaml]") {
    foeYamlException copy{"test"};
    foeYamlException test{std::move(copy)};

    REQUIRE(std::string{test.what()} == "test");
    REQUIRE(test.whatStr() == "test");
}