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
#include <foe/data_pool.hpp>

#include <memory>
#include <variant>

struct alignas(128) Custom {
    std::unique_ptr<int> test;
    std::variant<int, double> variant;

    Custom(int i) : test{new int}, variant{i} { *test = i; }

    bool operator==(int rhs) const noexcept {
        return test && *test == rhs && std::get<0>(variant) == rhs;
    }
};

using Pool = foeDataPool<uint32_t, Custom>;

template class foeDataPool<uint32_t, Custom>;

TEST_CASE("Pool<Custom> - Expansion Rate") {
    Pool test;

    SECTION("Default rate is 128") { CHECK(test.expansionRate() == 128); }

    SECTION("Rate can be changed") {
        test.expansionRate(8192);

        CHECK(test.expansionRate() == 8192);
    }
}

TEST_CASE("Pool<Custom> - Accessors on new object all have empty ranges") {
    Pool test;

    SECTION("regular accessors") {
        SECTION("non-const") {
            REQUIRE(test.begin() == test.end());
            REQUIRE(test.cbegin() == test.cend());

            REQUIRE(test.begin() == test.cbegin());
            REQUIRE(test.end() == test.cend());
        }
        SECTION("const") {
            Pool const &ctest = test;

            REQUIRE(ctest.begin() == ctest.end());
            REQUIRE(ctest.cbegin() == ctest.end());

            REQUIRE(ctest.begin() == ctest.cbegin());
            REQUIRE(ctest.end() == ctest.cend());
        }
    }

    SECTION("inserted accessors") {
        SECTION("non-const") {
            REQUIRE(test.inbegin() == test.inend());
            REQUIRE(test.cinbegin() == test.cinend());

            REQUIRE(test.inbegin() == test.cinbegin());
            REQUIRE(test.inend() == test.cinend());
        }
        SECTION("const") {
            Pool const &ctest = test;

            REQUIRE(ctest.inbegin() == ctest.inend());
            REQUIRE(ctest.cinbegin() == ctest.inend());

            REQUIRE(ctest.inbegin() == ctest.cinbegin());
            REQUIRE(ctest.inend() == ctest.cinend());
        }
    }

    SECTION("removed accessors") {
        SECTION("non-const") {
            REQUIRE(test.rmbegin() == test.rmend());
            REQUIRE(test.crmbegin() == test.crmend());

            REQUIRE(test.rmbegin() == test.crmbegin());
            REQUIRE(test.rmend() == test.crmend());
        }
        SECTION("const") {
            Pool const &ctest = test;

            REQUIRE(ctest.rmbegin() == ctest.rmend());
            REQUIRE(ctest.crmbegin() == ctest.rmend());

            REQUIRE(ctest.rmbegin() == ctest.crmbegin());
            REQUIRE(ctest.rmend() == ctest.crmend());
        }
    }
}

TEST_CASE("Pool<Custom> - Maintenance with no changes does nothing") {
    Pool test;

    test.maintenance();

    CHECK(test.capacity() == 0);
    CHECK(test.size() == 0);
    CHECK(test.inserted() == 0);
    CHECK(test.removed() == 0);
}

/** INSERTIONS **/

TEST_CASE("Pool<Custom> - Single insertion") {
    Pool test;

    SECTION("Entity doesn't exist in pool before insertion") {
        REQUIRE(!test.exist(uint32_t(256)));
    }

    int temp = 128;
    test.insert(uint32_t(256), std::move(temp));

    test.maintenance();

    SECTION("Entity has been inserted during maintenance, exists, and is accessible") {
        REQUIRE(test.size() == 1);
        REQUIRE(test.inserted() == 1);
        REQUIRE(test.removed() == 0);

        REQUIRE(test.exist(uint32_t(256)));

        SECTION("regular accessors") {
            SECTION("non-const accessors") {
                REQUIRE(test.begin() + 1 == test.end());
                REQUIRE(test.cbegin() + 1 == test.cend());

                REQUIRE(test.begin() + 1 == test.cend());
                REQUIRE(test.cbegin() + 1 == test.end());

                REQUIRE(*test.begin() == uint32_t(256));
                REQUIRE(*test.cbegin() == uint32_t(256));

                REQUIRE(*test.begin<1>() == 128);
                REQUIRE(*test.cbegin<1>() == 128);
            }
            SECTION("const accessors") {
                Pool const &ctest = test;

                REQUIRE(ctest.begin() + 1 == ctest.end());
                REQUIRE(ctest.cbegin() + 1 == ctest.cend());

                REQUIRE(ctest.begin() + 1 == ctest.cend());
                REQUIRE(ctest.cbegin() + 1 == ctest.end());

                REQUIRE(*ctest.begin() == uint32_t(256));
                REQUIRE(*ctest.cbegin() == uint32_t(256));

                REQUIRE(*ctest.begin<1>() == 128);
                REQUIRE(*ctest.cbegin<1>() == 128);
            }
        }

        SECTION("inserted accessors") {
            SECTION("non-const accessors") {
                REQUIRE(test.inbegin() + 1 == test.inend());
                REQUIRE(test.cinbegin() + 1 == test.cinend());

                REQUIRE(test.inbegin() + 1 == test.cinend());
                REQUIRE(test.cinbegin() + 1 == test.inend());

                REQUIRE(*(test.begin() + *test.inbegin()) == uint32_t(256));
                REQUIRE(*(test.cbegin() + *test.inbegin()) == uint32_t(256));

                REQUIRE(*(test.begin<1>() + *test.inbegin()) == 128);
                REQUIRE(*(test.begin<1>() + *test.inbegin()) == 128);
            }
            SECTION("const accessors") {
                Pool const &ctest = test;

                REQUIRE(ctest.inbegin() + 1 == ctest.inend());
                REQUIRE(ctest.cinbegin() + 1 == ctest.cinend());

                REQUIRE(ctest.inbegin() + 1 == ctest.cinend());
                REQUIRE(ctest.cinbegin() + 1 == ctest.inend());

                REQUIRE(*(ctest.begin() + *ctest.inbegin()) == uint32_t(256));
                REQUIRE(*(ctest.cbegin() + *ctest.inbegin()) == uint32_t(256));

                REQUIRE(*(ctest.begin<1>() + *ctest.inbegin()) == 128);
                REQUIRE(*(ctest.begin<1>() + *ctest.inbegin()) == 128);
            }
        }
    }
}

TEST_CASE("Pool<Custom> - Multiple reversed insertion") {
    Pool test;

    constexpr int cTestNum = 3;

    SECTION("Entity doesn't exist in pool before insertion") {
        for (int i = cTestNum - 1; i >= 0; --i) {
            REQUIRE(!test.exist(uint32_t(i)));
        }
    }

    for (int i = cTestNum - 1; i >= 0; --i) {
        int val = i;
        test.insert(uint32_t(i), std::move(val));
    }

    test.maintenance();

    SECTION("Entity has been inserted during maintenance, exists, and is accessible") {
        REQUIRE(test.size() == cTestNum);
        REQUIRE(test.inserted() == cTestNum);
        REQUIRE(test.removed() == 0);

        for (int i = 0; i < cTestNum; ++i) {
            REQUIRE(test.exist(uint32_t(i)));
        }

        SECTION("regular Accessors") {
            SECTION("non-const accessors") {
                REQUIRE(test.begin() + cTestNum == test.end());
                REQUIRE(test.cbegin() + cTestNum == test.cend());

                REQUIRE(test.begin() + cTestNum == test.cend());
                REQUIRE(test.cbegin() + cTestNum == test.end());

                auto idIt = test.begin();
                auto dataIt = test.begin<1>();
                auto cIdIt = test.cbegin();
                auto cDataIt = test.begin<1>();

                for (int i = 0; i < 3; ++i, ++idIt, ++dataIt, ++cIdIt, ++cDataIt) {
                    REQUIRE(*idIt == uint32_t(i));
                    REQUIRE(*cIdIt == uint32_t(i));

                    REQUIRE(*(test.begin<1>() + i) == i);
                    REQUIRE(*(test.cbegin<1>() + i) == i);

                    REQUIRE(*dataIt == i);
                    REQUIRE(*cDataIt == i);
                }
            }
            SECTION("const accessors") {
                Pool const &ctest = test;

                REQUIRE(ctest.begin() + cTestNum == ctest.end());
                REQUIRE(ctest.cbegin() + cTestNum == ctest.cend());

                REQUIRE(ctest.begin() + cTestNum == ctest.cend());
                REQUIRE(ctest.cbegin() + cTestNum == ctest.end());

                auto idIt = test.begin();
                auto dataIt = test.begin<1>();
                auto cIdIt = test.cbegin();
                auto cDataIt = test.begin<1>();

                for (int i = 0; i < 3; ++i, ++idIt, ++dataIt, ++cIdIt, ++cDataIt) {
                    REQUIRE(*idIt == uint32_t(i));
                    REQUIRE(*cIdIt == uint32_t(i));

                    REQUIRE(*(test.begin<1>() + i) == i);
                    REQUIRE(*(test.cbegin<1>() + i) == i);

                    REQUIRE(*dataIt == i);
                    REQUIRE(*cDataIt == i);
                }
            }
        }

        SECTION("inserted Accessors") {
            SECTION("non-const accessors") {
                REQUIRE(test.inbegin() + cTestNum == test.inend());
                REQUIRE(test.cinbegin() + cTestNum == test.cinend());

                REQUIRE(test.inbegin() + cTestNum == test.cinend());
                REQUIRE(test.cinbegin() + cTestNum == test.inend());

                auto it = test.inbegin();
                auto cit = test.cinbegin();

                for (int i = 0; i < cTestNum; ++i, ++it, ++cit) {
                    REQUIRE(*(test.begin() + *test.inbegin() + i) == i);
                    REQUIRE(*(test.cbegin() + *test.cinbegin() + i) == i);

                    REQUIRE(*(test.begin() + *it) == uint32_t(i));
                    REQUIRE(*(test.cbegin() + *cit) == uint32_t(i));

                    REQUIRE(*(test.begin<1>() + *it) == i);
                    REQUIRE(*(test.cbegin<1>() + *cit) == i);
                }
            }
            SECTION("const accessors") {
                Pool const &ctest = test;

                REQUIRE(ctest.inbegin() + cTestNum == ctest.inend());
                REQUIRE(ctest.cinbegin() + cTestNum == ctest.cinend());

                REQUIRE(ctest.inbegin() + cTestNum == ctest.cinend());
                REQUIRE(ctest.cinbegin() + cTestNum == ctest.inend());

                auto it = test.inbegin();
                auto cit = test.cinbegin();

                for (int i = 0; i < cTestNum; ++i, ++it, ++cit) {
                    REQUIRE(*(test.begin() + *test.inbegin() + i) == i);
                    REQUIRE(*(test.cbegin() + *test.cinbegin() + i) == i);

                    REQUIRE(*(test.begin() + *it) == uint32_t(i));
                    REQUIRE(*(test.cbegin() + *cit) == uint32_t(i));

                    REQUIRE(*(test.begin<1>() + *it) == i);
                    REQUIRE(*(test.cbegin<1>() + *cit) == i);
                }
            }
        }
    }
}

TEST_CASE("Pool<Custom> - Testing insert/regular storage expansion") {
    // Starting the pool with essentiall no storage, and inserting more than the storage
    // initially holds to test that the expansion mechanisms work correctly.
    Pool test;

    constexpr int cTestNum = 3;

    SECTION("Entity doesn't exist in pool before insertion") {
        for (int i = cTestNum - 1; i >= 0; --i) {
            REQUIRE(!test.exist(uint32_t(i)));
        }
    }

    for (int i = cTestNum - 1; i >= 0; --i) {
        int val = i;
        test.insert(uint32_t(i), std::move(val));
    }

    test.maintenance();

    REQUIRE(test.capacity() >= cTestNum);

    SECTION("Entity has been inserted during maintenance, exists, and is accessible") {
        REQUIRE(test.size() == cTestNum);
        REQUIRE(test.inserted() == cTestNum);
        REQUIRE(test.removed() == 0);

        for (int i = 0; i < cTestNum; ++i) {
            REQUIRE(test.exist(uint32_t(i)));
        }

        SECTION("regular Accessors") {
            SECTION("non-const accessors") {
                REQUIRE(test.begin() + cTestNum == test.end());
                REQUIRE(test.cbegin() + cTestNum == test.cend());

                REQUIRE(test.begin() + cTestNum == test.cend());
                REQUIRE(test.cbegin() + cTestNum == test.end());

                auto idIt = test.begin();
                auto dataIt = test.begin<1>();
                auto cIdIt = test.cbegin();
                auto cDataIt = test.begin<1>();

                for (int i = 0; i < 3; ++i, ++idIt, ++dataIt, ++cIdIt, ++cDataIt) {
                    REQUIRE(*idIt == uint32_t(i));
                    REQUIRE(*cIdIt == uint32_t(i));

                    REQUIRE(*(test.begin<1>() + i) == i);
                    REQUIRE(*(test.cbegin<1>() + i) == i);

                    REQUIRE(*dataIt == i);
                    REQUIRE(*cDataIt == i);
                }
            }
            SECTION("const accessors") {
                Pool const &ctest = test;

                REQUIRE(ctest.begin() + cTestNum == ctest.end());
                REQUIRE(ctest.cbegin() + cTestNum == ctest.cend());

                REQUIRE(ctest.begin() + cTestNum == ctest.cend());
                REQUIRE(ctest.cbegin() + cTestNum == ctest.end());

                auto idIt = test.begin();
                auto dataIt = test.begin<1>();
                auto cIdIt = test.cbegin();
                auto cDataIt = test.begin<1>();

                for (int i = 0; i < 3; ++i, ++idIt, ++dataIt, ++cIdIt, ++cDataIt) {
                    REQUIRE(*idIt == uint32_t(i));
                    REQUIRE(*cIdIt == uint32_t(i));

                    REQUIRE(*(test.begin<1>() + i) == i);
                    REQUIRE(*(test.cbegin<1>() + i) == i);

                    REQUIRE(*dataIt == i);
                    REQUIRE(*cDataIt == i);
                }
            }
        }

        SECTION("inserted Accessors") {
            SECTION("non-const accessors") {
                REQUIRE(test.inbegin() + cTestNum == test.inend());
                REQUIRE(test.cinbegin() + cTestNum == test.cinend());

                REQUIRE(test.inbegin() + cTestNum == test.cinend());
                REQUIRE(test.cinbegin() + cTestNum == test.inend());

                auto it = test.inbegin();
                auto cit = test.cinbegin();

                for (int i = 0; i < cTestNum; ++i, ++it, ++cit) {
                    REQUIRE(*(test.begin() + *test.inbegin() + i) == i);
                    REQUIRE(*(test.cbegin() + *test.cinbegin() + i) == i);

                    REQUIRE(*(test.begin() + *it) == uint32_t(i));
                    REQUIRE(*(test.cbegin() + *cit) == uint32_t(i));

                    REQUIRE(*(test.begin<1>() + *it) == i);
                    REQUIRE(*(test.cbegin<1>() + *cit) == i);
                }
            }
            SECTION("const accessors") {
                Pool const &ctest = test;

                REQUIRE(ctest.inbegin() + cTestNum == ctest.inend());
                REQUIRE(ctest.cinbegin() + cTestNum == ctest.cinend());

                REQUIRE(ctest.inbegin() + cTestNum == ctest.cinend());
                REQUIRE(ctest.cinbegin() + cTestNum == ctest.inend());

                auto it = test.inbegin();
                auto cit = test.cinbegin();

                for (int i = 0; i < cTestNum; ++i, ++it, ++cit) {
                    REQUIRE(*(test.begin() + *test.inbegin() + i) == i);
                    REQUIRE(*(test.cbegin() + *test.cinbegin() + i) == i);

                    REQUIRE(*(test.begin() + *it) == uint32_t(i));
                    REQUIRE(*(test.cbegin() + *cit) == uint32_t(i));

                    REQUIRE(*(test.begin<1>() + *it) == i);
                    REQUIRE(*(test.cbegin<1>() + *cit) == i);
                }
            }
        }
    }
}

TEST_CASE("Pool<Custom> - Staggered insertion") {
    // In this case, we'll first insert a group of even numbers in a pass, then in the next pass,
    // the interleaving set of odd numbers, ensuring that they interleave and sort out
    // correctly.
    Pool test; //{32};
    constexpr int cTestNum = 20;

    for (int i = cTestNum; i >= 0; i -= 2) {
        int val = i;
        test.insert(uint32_t(i), std::move(val));
    }

    test.maintenance();

    SECTION("even items should exist, and in ascending order") {
        auto idIt = test.begin();
        auto dataIt = test.begin();
        auto inIt = test.inbegin();

        for (int i = 0; i < cTestNum; i += 2, ++idIt, ++dataIt, ++inIt) {
            REQUIRE(*idIt == uint32_t(i));
            REQUIRE(*dataIt == i);

            REQUIRE(*(test.begin() + *inIt) == uint32_t(i));
            REQUIRE(*(test.begin<1>() + *inIt) == i);
        }
    }
    SECTION("off numbers don't exist yet") {
        for (int i = 1; i < cTestNum; i += 2) {
            REQUIRE(!test.exist(uint32_t(i)));
        }
    }

    // Inserting the odd numbers
    for (int i = cTestNum - 1; i >= 0; i -= 2) {
        int val = i;
        test.insert(uint32_t(i), std::move(val));
    }

    test.maintenance();

    SECTION("all numbers exist, in correct order") {
        auto idIt = test.begin();
        auto dataIt = test.begin();

        for (int i = 0; i < cTestNum; ++i, ++idIt, ++dataIt) {
            REQUIRE(*idIt == uint32_t(i));
            REQUIRE(*dataIt == i);
        }
    }
    SECTION("odd numbered insertions are the only ones in the insertion list, in order") {
        auto it = test.inbegin();

        for (int i = 1; i < cTestNum; i += 2, ++it) {
            REQUIRE(*(test.begin() + *it) == uint32_t(i));
            REQUIRE(*(test.begin<1>() + *it) == i);
        }
    }
}

TEST_CASE(
    "Pool<Custom> - Attempting to insert multiple of the same uint32_t fails, only the *last* one "
    "is inserted") {
    Pool test;

    int temp = 1;
    test.insert(uint32_t(16), std::move(temp));
    temp = 2;
    test.insert(uint32_t(16), std::move(temp));
    temp = 3;
    test.insert(uint32_t(16), std::move(temp));

    test.maintenance();

    REQUIRE(test.size() == 1);
    REQUIRE(test.inserted() == 1);

    REQUIRE(*test.begin() == uint32_t(16));
    REQUIRE(*test.begin<1>() == 3);
}

TEST_CASE("Pool<Custom> - Attempting to add same entity in a different pass fails, original stays "
          "intact") {
    Pool test;

    int temp = 1;
    test.insert(uint32_t(16), std::move(temp));
    test.maintenance();

    temp = 2;
    test.insert(uint32_t(16), std::move(temp));
    test.maintenance();

    REQUIRE(test.size() == 1);
    REQUIRE(test.inserted() == 0);

    REQUIRE(*test.begin() == uint32_t(16));
    REQUIRE(*test.begin<1>() == 1);
}

// REMOVALS

TEST_CASE("Pool<Custom> - Single removal") {
    Pool test;

    int temp = 128;
    test.insert(uint32_t(256), std::move(temp));

    test.maintenance();

    REQUIRE(test.size() == 1);
    REQUIRE(test.inserted() == 1);
    REQUIRE(test.removed() == 0);
    REQUIRE(test.exist(uint32_t(256)));

    test.remove(uint32_t(256));

    test.maintenance();

    SECTION("Entity has been removed during maintenance, no longer exists, and is accessible") {
        REQUIRE(test.size() == 0);
        REQUIRE(test.inserted() == 0);
        REQUIRE(test.removed() == 1);

        REQUIRE(!test.exist(uint32_t(256)));

        SECTION("removed accessors") {
            SECTION("non-const accessors") {
                REQUIRE(test.rmbegin() + 1 == test.rmend());
                REQUIRE(test.crmbegin() + 1 == test.crmend());

                REQUIRE(test.rmbegin() + 1 == test.crmend());
                REQUIRE(test.crmbegin() + 1 == test.rmend());

                REQUIRE(*test.rmbegin() == uint32_t(256));
                REQUIRE(*test.crmbegin() == uint32_t(256));

                REQUIRE(*test.rmbegin<1>() == 128);
                REQUIRE(*test.crmbegin<1>() == 128);
            }
            SECTION("const accessors") {
                Pool const &ctest = test;

                REQUIRE(ctest.rmbegin() + 1 == ctest.rmend());
                REQUIRE(ctest.crmbegin() + 1 == ctest.crmend());

                REQUIRE(ctest.rmbegin() + 1 == ctest.crmend());
                REQUIRE(ctest.crmbegin() + 1 == ctest.rmend());

                REQUIRE(*ctest.rmbegin() == uint32_t(256));
                REQUIRE(*ctest.crmbegin() == uint32_t(256));

                REQUIRE(*ctest.rmbegin<1>() == 128);
                REQUIRE(*ctest.crmbegin<1>() == 128);
            }
        }
    }
}

TEST_CASE("Pool<Custom> - Multiple removal") {
    Pool test;
    constexpr int cTestNum = 20;

    for (int i = 0; i < cTestNum; ++i) {
        int val = i;
        test.insert(uint32_t(i), std::move(val));
    }

    test.maintenance();

    REQUIRE(test.size() == 20);

    for (int i = cTestNum; i >= 0; --i) {
        test.remove(uint32_t(i));
    }

    test.maintenance();

    SECTION("Entities have been removed during maintenance, no longer exists, and is accessible") {
        REQUIRE(test.size() == 0);
        REQUIRE(test.removed() == 20);

        SECTION("removed accessors") {
            SECTION("non-const accessors") {
                REQUIRE(test.rmbegin() + cTestNum == test.rmend());
                REQUIRE(test.crmbegin() + cTestNum == test.crmend());

                REQUIRE(test.rmbegin() + cTestNum == test.crmend());
                REQUIRE(test.crmbegin() + cTestNum == test.rmend());

                auto idIt = test.rmbegin();
                auto dataIt = test.rmbegin<1>();
                auto cIdIt = test.crmbegin();
                auto cDataIt = test.crmbegin<1>();

                for (int i = 0; i < cTestNum; ++i, ++idIt, ++dataIt, ++cIdIt, ++cDataIt) {
                    REQUIRE(*(test.rmbegin() + i) == i);
                    REQUIRE(*(test.crmbegin() + i) == i);

                    REQUIRE(*idIt == uint32_t(i));
                    REQUIRE(*cIdIt == uint32_t(i));

                    REQUIRE(*dataIt == i);
                    REQUIRE(*cDataIt == i);
                }
            }
            SECTION("const accessors") {
                Pool const &ctest = test;

                REQUIRE(ctest.rmbegin() + cTestNum == ctest.rmend());
                REQUIRE(ctest.crmbegin() + cTestNum == ctest.crmend());

                REQUIRE(ctest.rmbegin() + cTestNum == ctest.crmend());
                REQUIRE(ctest.crmbegin() + cTestNum == ctest.rmend());

                auto idIt = test.rmbegin();
                auto dataIt = test.rmbegin<1>();
                auto cIdIt = test.crmbegin();
                auto cDataIt = test.crmbegin<1>();

                for (int i = 0; i < cTestNum; ++i, ++idIt, ++dataIt, ++cIdIt, ++cDataIt) {
                    REQUIRE(*(test.rmbegin() + i) == i);
                    REQUIRE(*(test.crmbegin() + i) == i);

                    REQUIRE(*idIt == uint32_t(i));
                    REQUIRE(*cIdIt == uint32_t(i));

                    REQUIRE(*dataIt == i);
                    REQUIRE(*cDataIt == i);
                }
            }
        }
    }
}

TEST_CASE("Pool<Custom> - Staggered removal") {
    // All items are added, then just the odd ones are removed, making sure that removals are
    // correct.
    Pool test;
    constexpr int cTestNum = 20;

    for (int i = 0; i < cTestNum; ++i) {
        int val = i;
        test.insert(uint32_t(i), std::move(val));
    }

    test.maintenance();

    REQUIRE(test.size() == cTestNum);
    REQUIRE(test.inserted() == cTestNum);
    REQUIRE(test.removed() == 0);

    for (int i = cTestNum - 1; i >= 0; i -= 2) {
        test.remove(uint32_t(i));
    }

    test.maintenance();

    REQUIRE(test.size() == cTestNum / 2);
    REQUIRE(test.inserted() == 0);
    REQUIRE(test.removed() == cTestNum / 2);

    SECTION("Entities have been removed during maintenance, no longer exists, and is accessible") {
        auto it = test.rmbegin();
        auto dataIt = test.rmbegin<1>();

        for (int i = 1; i < cTestNum; i += 2, ++it, ++dataIt) {
            REQUIRE(!test.exist(uint32_t(i)));
            REQUIRE(*it == uint32_t(i));
            REQUIRE(*dataIt == i);
        }
    }
    SECTION("Entities that should remain do so, and in order") {
        auto it = test.begin();
        auto dataIt = test.begin<1>();

        for (int i = 0; i < cTestNum; i += 2, ++it, ++dataIt) {
            REQUIRE(test.exist(uint32_t(i)));
            REQUIRE(*it == uint32_t(i));
            REQUIRE(*dataIt == i);
        }
    }
}

TEST_CASE(
    "Pool<Custom> - Attempting to remove an item multiple times doesn't have undesired effects") {
    Pool test;

    int temp = 10;
    test.insert(uint32_t(10), std::move(temp));
    temp = 12;
    test.insert(uint32_t(12), std::move(temp));
    temp = 8;
    test.insert(uint32_t(8), std::move(temp));

    test.maintenance();

    REQUIRE(test.size() == 3);

    test.remove(uint32_t(10));
    test.remove(uint32_t(10));

    test.maintenance();

    REQUIRE(test.size() == 2);
    REQUIRE(test.removed() == 1);

    SECTION("Remaining items") {
        REQUIRE(*test.begin() == uint32_t(8));
        REQUIRE(*test.begin<1>() == 8);

        REQUIRE(*(test.begin() + 1) == uint32_t(12));
        REQUIRE(*(test.begin<1>() + 1) == 12);
    }
    SECTION("Removed item") {
        REQUIRE(*(test.rmbegin()) == uint32_t(10));
        REQUIRE(*(test.rmbegin<1>()) == 10);
    }
}

TEST_CASE("Pool<Custom> - Attempting to remove items not in the pool has no undesired effects") {
    Pool test;

    int temp = 1;
    test.insert(uint32_t(1), std::move(temp));
    temp = 3;
    test.insert(uint32_t(3), std::move(temp));
    temp = 5;
    test.insert(uint32_t(5), std::move(temp));

    test.maintenance();

    test.remove(uint32_t(0));
    test.remove(uint32_t(2));
    test.remove(uint32_t(4));
    test.remove(uint32_t(6));

    test.maintenance();

    REQUIRE(test.size() == 3);
    REQUIRE(test.inserted() == 0);
    REQUIRE(test.removed() == 0);
}

TEST_CASE("Pool<Custom> - Search/find functionality") {
    Pool test;
    constexpr int cBuffer = 16;
    constexpr int cTestNum = 16;

    for (int i = cBuffer; i < cBuffer + cTestNum; ++i) {
        int val = i;
        test.insert(uint32_t(i), std::move(val));
    }

    test.maintenance();

    REQUIRE(test.size() == cTestNum);
    REQUIRE(test.inserted() == cTestNum);

    SECTION("Pre-buffer can't be found") {
        for (int i = 0; i < cBuffer; ++i) {
            REQUIRE(test.sequential_search(uint32_t(i)) == test.size());
            REQUIRE(test.binary_search(uint32_t(i)) == test.size());
            REQUIRE(test.find(uint32_t(i)) == test.size());

            auto const &ctest = test;

            REQUIRE(ctest.sequential_search(uint32_t(i)) == test.size());
            REQUIRE(ctest.binary_search(uint32_t(i)) == test.size());
            REQUIRE(ctest.find(uint32_t(i)) == test.size());
        }
    }
    SECTION("Inserted items can be found") {
        for (int i = cBuffer; i < cBuffer + cTestNum; ++i) {
            REQUIRE(test.sequential_search(uint32_t(i)) != test.size());
            REQUIRE(test.binary_search(uint32_t(i)) != test.size());
            REQUIRE(test.find(uint32_t(i)) != test.size());

            auto const &ctest = test;

            REQUIRE(ctest.sequential_search(uint32_t(i)) != test.size());
            REQUIRE(ctest.binary_search(uint32_t(i)) != test.size());
            REQUIRE(ctest.find(uint32_t(i)) != test.size());
        }
    }
    SECTION("Post-buffer can't be found") {
        for (int i = cBuffer + cTestNum; i < cBuffer * 2 + cTestNum; ++i) {
            REQUIRE(test.sequential_search(uint32_t(i)) == test.size());
            REQUIRE(test.binary_search(uint32_t(i)) == test.size());
            REQUIRE(test.find(uint32_t(i)) == test.size());

            auto const &ctest = test;

            REQUIRE(ctest.sequential_search(uint32_t(i)) == test.size());
            REQUIRE(ctest.binary_search(uint32_t(i)) == test.size());
            REQUIRE(ctest.find(uint32_t(i)) == test.size());
        }
    }

    SECTION("removed search") {
        for (int i = cBuffer; i < cBuffer + cTestNum; ++i) {
            test.remove(uint32_t(i));
        }

        test.maintenance();

        REQUIRE(test.size() == 0);
        REQUIRE(test.removed() == cTestNum);

        SECTION("Pre-buffer can't be found") {
            for (int i = 0; i < cBuffer; ++i) {
                REQUIRE(test.rm_sequential_search(uint32_t(i)) == test.removed());
                REQUIRE(test.rm_binary_search(uint32_t(i)) == test.removed());
                REQUIRE(test.rm_find(uint32_t(i)) == test.removed());

                auto const &ctest = test;

                REQUIRE(ctest.rm_sequential_search(uint32_t(i)) == test.removed());
                REQUIRE(ctest.rm_binary_search(uint32_t(i)) == test.removed());
                REQUIRE(ctest.rm_find(uint32_t(i)) == test.removed());
            }
        }
        SECTION("Inserted items can't be found") {
            for (int i = cBuffer; i < cBuffer + cTestNum; ++i) {
                REQUIRE(test.rm_sequential_search(uint32_t(i)) != test.removed());
                REQUIRE(test.rm_binary_search(uint32_t(i)) != test.removed());
                REQUIRE(test.rm_find(uint32_t(i)) != test.removed());

                auto const &ctest = test;

                REQUIRE(ctest.rm_sequential_search(uint32_t(i)) != test.removed());
                REQUIRE(ctest.rm_binary_search(uint32_t(i)) != test.removed());
                REQUIRE(ctest.rm_find(uint32_t(i)) != test.removed());
            }
        }
        SECTION("Post-buffer can't be found") {
            for (int i = cBuffer + cTestNum; i < cBuffer * 2 + cTestNum; ++i) {
                REQUIRE(test.rm_sequential_search(uint32_t(i)) == test.removed());
                REQUIRE(test.rm_binary_search(uint32_t(i)) == test.removed());
                REQUIRE(test.rm_find(uint32_t(i)) == test.removed());

                auto const &ctest = test;

                REQUIRE(ctest.rm_sequential_search(uint32_t(i)) == test.removed());
                REQUIRE(ctest.rm_binary_search(uint32_t(i)) == test.removed());
                REQUIRE(ctest.rm_find(uint32_t(i)) == test.removed());
            }
        }
    }
}
