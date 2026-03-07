// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/search_paths.hpp>

#include <string_view>

TEST_CASE("Default constructor") {
    foeSearchPaths test;

    auto reader = test.getReader();
    REQUIRE(reader.searchPaths()->empty());
}

TEST_CASE("Initializing constructor constructor") {
    SECTION("Initializer list") {
        foeSearchPaths test{{"/lol"}};

        auto reader = test.getReader();
        REQUIRE(reader.searchPaths()->size() == 1);
        REQUIRE((*reader.searchPaths())[0] == "/lol");
    }
    SECTION("Copied vector") {
        std::vector<std::filesystem::path> temp = {"/lol"};
        foeSearchPaths test{temp};

        auto reader = test.getReader();
        REQUIRE(reader.searchPaths()->size() == 1);
        REQUIRE((*reader.searchPaths())[0] == "/lol");
    }
    SECTION("Moved vector") {
        std::vector<std::filesystem::path> temp = {"/lol"};
        foeSearchPaths test{std::move(temp)};

        auto reader = test.getReader();
        REQUIRE(reader.searchPaths()->size() == 1);
        REQUIRE((*reader.searchPaths())[0] == "/lol");
    }
}

TEST_CASE("SearchPaths - Read lock", "[foe][SearchPaths]") {
    foeSearchPaths test;

    auto reader = test.getReader();
    REQUIRE(reader.valid());
    REQUIRE(reader.searchPaths() != nullptr);

    SECTION("Move Constructor") {
        auto newReader{std::move(reader)};

        REQUIRE(!reader.valid());
        REQUIRE(reader.searchPaths() == nullptr);

        REQUIRE(newReader.valid());
        REQUIRE(newReader.searchPaths() != nullptr);
    }
    SECTION("Move operator") {
        foeSearchPaths::Reader newReader;
        newReader = std::move(reader);

        REQUIRE(!reader.valid());
        REQUIRE(reader.searchPaths() == nullptr);

        REQUIRE(newReader.valid());
        REQUIRE(newReader.searchPaths() != nullptr);

        SECTION("Moving to current properly releases lock") {
            newReader = foeSearchPaths::Reader{};

            auto writer = test.tryGetWriter();
            REQUIRE(writer.valid());
        }
    }

    SECTION("Getting more than one reader succeeds") {
        auto reader2 = test.tryGetReader();
        REQUIRE(reader2.valid());
        REQUIRE(reader.searchPaths() == reader2.searchPaths());

        auto reader3 = test.tryGetReader();
        REQUIRE(reader3.valid());
        REQUIRE(reader.searchPaths() == reader3.searchPaths());
    }
    SECTION("Getting a write lock when read locked fails") {
        auto writer = test.tryGetWriter();

        REQUIRE(!writer.valid());
        REQUIRE(writer.searchPaths() == nullptr);
    }
    SECTION("Releasing via release() invalidates the accessor") {
        reader.release();
        REQUIRE(!reader.valid());
        REQUIRE(reader.searchPaths() == nullptr);

        SECTION("After releasing the reader, write lock can be attained") {
            auto writer = test.getWriter();
            REQUIRE(writer.valid());
            REQUIRE(writer.searchPaths() != nullptr);
        }
    }
}

TEST_CASE("SearchPath - Write lock", "[foe][SearchPaths]") {
    foeSearchPaths test;

    auto writer = test.getWriter();
    REQUIRE(writer.valid());
    REQUIRE(writer.searchPaths() != nullptr);

    SECTION("Move Constructor") {
        auto newWriter{std::move(writer)};

        REQUIRE(!writer.valid());
        REQUIRE(writer.searchPaths() == nullptr);

        REQUIRE(newWriter.valid());
        REQUIRE(newWriter.searchPaths() != nullptr);
    }
    SECTION("Move operator") {
        foeSearchPaths::Writer newWriter;
        newWriter = std::move(writer);

        REQUIRE(!writer.valid());
        REQUIRE(writer.searchPaths() == nullptr);

        REQUIRE(newWriter.valid());
        REQUIRE(newWriter.searchPaths() != nullptr);

        SECTION("Moving to current properly releases lock") {
            newWriter = foeSearchPaths::Writer{};

            auto reader = test.tryGetReader();
            REQUIRE(reader.valid());
        }
    }

    SECTION("Getting more than one writer fails") {
        auto writer2 = test.tryGetWriter();
        REQUIRE(!writer2.valid());
        REQUIRE(writer2.searchPaths() == nullptr);
    }
    SECTION("Getting a read lock when write locked fails") {
        auto reader = test.tryGetReader();
        REQUIRE(!reader.valid());
    }
    SECTION("Releasing via release() invalidates the accessor") {
        writer.release();
        REQUIRE(!writer.valid());
        REQUIRE(writer.searchPaths() == nullptr);

        SECTION("After releasing the writer, read locks can be attained") {
            auto reader = test.getReader();
            REQUIRE(reader.valid());
            REQUIRE(reader.searchPaths() != nullptr);
        }
        SECTION("After releasing, another write lock can be attained") {
            auto writer = test.tryGetWriter();
            REQUIRE(writer.valid());
            REQUIRE(writer.searchPaths() != nullptr);
        }
    }
}

TEST_CASE("SearchPaths - Writing, then reading the path list", "[foe][SearchPaths]") {
    foeSearchPaths test;

    std::vector<std::filesystem::path> testList = {"foo", "bar"};

    auto writer = test.getWriter();
    *writer.searchPaths() = testList;
    writer.release();

    auto reader = test.getReader();
    auto const *readList = reader.searchPaths();
    REQUIRE(readList->size() == testList.size());
    for (std::size_t i = 0; i < testList.size(); ++i) {
        REQUIRE((*readList)[i] == testList[i]);
    }
}
