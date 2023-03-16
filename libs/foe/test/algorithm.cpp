// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/algorithm.hpp>

namespace {

// Testing structure
struct testStruct {
    uint32_t first;
    uint32_t second;
};

// Comparator function
auto pred = [](auto &lhs, auto &rhs) { return lhs.first == rhs.first; };

auto intPred = [](auto &lhs, auto &rhs) { return lhs == rhs; };

} // namespace

TEST_CASE("unique_last<testStruct> - Test on an empty list", "[foe][unique_last]") {
    std::vector<testStruct> test = {};

    SECTION("does nothing and returns the end iterator provided") {
        auto newEnd = foe::unique_last(test.begin(), test.end(), pred);
        REQUIRE(newEnd == test.begin());
    }
}

TEST_CASE("unique_last<testStruct> - Test objects in the middle of the list",
          "[foe][unique_last]") {
    std::vector<testStruct> test = {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {3, 1}, {4, 0}, {4, 1}, {5, 0}};

    REQUIRE(test.size() == 8);

    SECTION("std::unique only provides first of sequence objects back", "[foe][unique_last]") {
        auto newEnd = std::unique(test.begin(), test.end(), pred);

        test.erase(newEnd, test.end());

        REQUIRE(test.size() == 6);
        REQUIRE(test[3].first == 3);
        REQUIRE(test[3].second == 0);
        REQUIRE(test[4].first == 4);
        REQUIRE(test[4].second == 0);
    }
    SECTION("foe::unique_last leaves the last of sequence objects", "[foe][unique_last]") {
        auto newEnd = foe::unique_last(test.begin(), test.end(), pred);

        test.erase(newEnd, test.end());

        REQUIRE(test.size() == 6);
        REQUIRE(test[3].first == 3);
        REQUIRE(test[3].second == 1);
        REQUIRE(test[4].first == 4);
        REQUIRE(test[4].second == 1);
    }
}

TEST_CASE("unique_last<testStruct> - Test objects at the start/end of the list",
          "[foe][unique_last]") {
    std::vector<testStruct> test = {
        {0, 0}, {0, 1}, {0, 2}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {5, 1}, {5, 2},
    };

    REQUIRE(test.size() == 10);

    SECTION("leaves the last of each sequence in the container") {
        auto newEnd = foe::unique_last(test.begin(), test.end(), pred);

        test.erase(newEnd, test.end());

        REQUIRE(test.size() == 6);
        REQUIRE(test[0].first == 0);
        REQUIRE(test[0].second == 2);
        REQUIRE(test[5].first == 5);
        REQUIRE(test[5].second == 2);
    }
}

TEST_CASE("unique_last<int> - Test on an empty list", "[foe]") {
    std::vector<int> test = {};

    SECTION("unique_last does nothing and returns the end iterator provided") {
        SECTION("without predicate") {
            auto newEnd = foe::unique_last(test.begin(), test.end());
            REQUIRE(newEnd == test.begin());
        }
        SECTION("with predicate") {
            auto newEnd = foe::unique_last(test.begin(), test.end(), intPred);
            REQUIRE(newEnd == test.begin());
        }
    }
    SECTION("remove_duplicates does nothing and returns the end iterator provided") {
        auto newEnd = foe::remove_duplicates(test.begin(), test.end());
        REQUIRE(newEnd == test.begin());
    }
}

TEST_CASE("unique_last<int> - Test objects in the middle of the list", "[foe]") {
    std::vector<int> test = {0, 1, 2, 3, 3, 4, 4, 5};

    REQUIRE(test.size() == 8);

    SECTION("unique_last") {
        SECTION("without predicate") {
            auto newEnd = foe::unique_last(test.begin(), test.end());
            test.erase(newEnd, test.end());

            REQUIRE(test == std::vector<int>{0, 1, 2, 3, 4, 5});
        }
        SECTION("with predicate") {
            auto newEnd = foe::unique_last(test.begin(), test.end(), intPred);
            test.erase(newEnd, test.end());
            REQUIRE(test == std::vector<int>{0, 1, 2, 3, 4, 5});
        }
    }
    SECTION("remove_duplicates") {
        auto newEnd = foe::remove_duplicates(test.begin(), test.end());
        test.erase(newEnd, test.end());
        REQUIRE(test == std::vector<int>{0, 1, 2, 3, 4, 5});
    }
}

TEST_CASE("unique_last<int> - Test objects at the start/end of the list", "[foe]") {
    std::vector<int> test = {0, 0, 0, 1, 2, 3, 4, 5, 5, 5, 5};

    REQUIRE(test.size() == 11);

    SECTION("unique_last") {
        SECTION("without predicate") {
            auto newEnd = foe::unique_last(test.begin(), test.end());
            test.erase(newEnd, test.end());

            REQUIRE(test == std::vector<int>{0, 1, 2, 3, 4, 5});
        }
        SECTION("with predicate") {
            auto newEnd = foe::unique_last(test.begin(), test.end(), intPred);
            test.erase(newEnd, test.end());
            REQUIRE(test == std::vector<int>{0, 1, 2, 3, 4, 5});
        }
    }
    SECTION("remove_duplicates") {
        auto newEnd = foe::remove_duplicates(test.begin(), test.end());
        test.erase(newEnd, test.end());
        REQUIRE(test == std::vector<int>{0, 1, 2, 3, 4, 5});
    }
}

TEST_CASE("unique_last<int> - Test objects spread throughout the list, out of order", "[foe]") {
    std::vector<int> test = {0, 1, 0, 2, 0, 3, 0, 4, 0, 5};

    REQUIRE(test.size() == 10);

    SECTION("unique_last") {
        SECTION("without predicate") {
            auto newEnd = foe::unique_last(test.begin(), test.end());
            test.erase(newEnd, test.end());

            REQUIRE(test == std::vector<int>{0, 1, 0, 2, 0, 3, 0, 4, 0, 5});
        }
        SECTION("with predicate") {
            auto newEnd = foe::unique_last(test.begin(), test.end(), intPred);
            test.erase(newEnd, test.end());
            REQUIRE(test == std::vector<int>{0, 1, 0, 2, 0, 3, 0, 4, 0, 5});
        }
    }
    SECTION("remove_duplicates") {
        auto newEnd = foe::remove_duplicates(test.begin(), test.end());
        test.erase(newEnd, test.end());
        REQUIRE(test == std::vector<int>{0, 1, 2, 3, 4, 5});
    }
}