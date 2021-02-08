/*
    Copyright (C) 2021 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

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