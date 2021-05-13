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

using Pool = foeDataPool<uint32_t, uint32_t, int, double>;

template class foeDataPool<uint32_t, uint32_t, int, double>;

TEST_CASE("Pool<MULTI> - Expansion Rate") {
    Pool test;

    SECTION("Default rate is 128") { CHECK(test.expansionRate() == 128); }

    SECTION("Rate can be changed") {
        test.expansionRate(8192);

        CHECK(test.expansionRate() == 8192);
    }
}

TEST_CASE("Pool<MULTI> - Accessors on new object all have empty ranges") {
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

TEST_CASE("Pool<MULTI> - Maintenance with no changes does nothing") {
    Pool test;

    test.maintenance();

    REQUIRE(test.capacity() == 0);
    REQUIRE(test.size() == 0);
    REQUIRE(test.inserted() == 0);
    REQUIRE(test.removed() == 0);
}

/** INSERTIONS **/

TEST_CASE("Pool<MULTI> - Single insertion") {
    Pool test;

    SECTION("Entity doesn't exist in pool before insertion") {
        REQUIRE(!test.exist(uint32_t(256)));
    }

    int tempI = 128;
    unsigned int tempUi = 128;
    double tempD = 128.;
    test.insert(uint32_t(256), std::move(tempI), std::move(tempUi), std::move(tempD));

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

                REQUIRE(*test.begin<2>() == 128);
                REQUIRE(*test.cbegin<2>() == 128);

                REQUIRE(*test.begin<3>() == 128.);
                REQUIRE(*test.cbegin<3>() == 128.);
            }
            SECTION("const accessors") {
                Pool const &ctest = test;

                REQUIRE(ctest.begin() + 1 == ctest.end());
                REQUIRE(ctest.cbegin() + 1 == ctest.cend());

                REQUIRE(ctest.begin() + 1 == ctest.cend());
                REQUIRE(ctest.cbegin() + 1 == ctest.end());

                REQUIRE(*test.begin() == uint32_t(256));
                REQUIRE(*test.cbegin() == uint32_t(256));

                REQUIRE(*test.begin<1>() == 128);
                REQUIRE(*test.cbegin<1>() == 128);

                REQUIRE(*test.begin<2>() == 128);
                REQUIRE(*test.cbegin<2>() == 128);

                REQUIRE(*test.begin<3>() == 128.);
                REQUIRE(*test.cbegin<3>() == 128.);
            }
        }

        SECTION("inserted accessors") {
            SECTION("non-const accessors") {
                REQUIRE(test.inbegin() + 1 == test.inend());
                REQUIRE(test.cinbegin() + 1 == test.cinend());

                REQUIRE(test.inbegin() + 1 == test.cinend());
                REQUIRE(test.cinbegin() + 1 == test.inend());

                REQUIRE(*test.begin() == uint32_t(256));
                REQUIRE(*test.cbegin() == uint32_t(256));

                REQUIRE(*test.begin<1>() == 128);
                REQUIRE(*test.cbegin<1>() == 128);

                REQUIRE(*test.begin<2>() == 128);
                REQUIRE(*test.cbegin<2>() == 128);

                REQUIRE(*test.begin<3>() == 128.);
                REQUIRE(*test.cbegin<3>() == 128.);
            }
            SECTION("const accessors") {
                Pool const &ctest = test;

                REQUIRE(ctest.inbegin() + 1 == ctest.inend());
                REQUIRE(ctest.cinbegin() + 1 == ctest.cinend());

                REQUIRE(ctest.inbegin() + 1 == ctest.cinend());
                REQUIRE(ctest.cinbegin() + 1 == ctest.inend());

                REQUIRE(*test.begin() == uint32_t(256));
                REQUIRE(*test.cbegin() == uint32_t(256));

                REQUIRE(*test.begin<1>() == 128);
                REQUIRE(*test.cbegin<1>() == 128);

                REQUIRE(*test.begin<2>() == 128);
                REQUIRE(*test.cbegin<2>() == 128);

                REQUIRE(*test.begin<3>() == 128.);
                REQUIRE(*test.cbegin<3>() == 128.);
            }
        }
    }
}

TEST_CASE("Pool<MULTI> - Multiple reversed insertion") {
    Pool test;

    constexpr int cTestNum = 3;

    SECTION("Entity doesn't exist in pool before insertion") {
        for (int i = cTestNum - 1; i >= 0; --i) {
            REQUIRE(!test.exist(uint32_t(i)));
        }
    }

    for (int i = cTestNum - 1; i >= 0; --i) {
        int tempI = i;
        unsigned int tempUi = i;
        double tempD = i;
        test.insert(uint32_t(i), std::move(tempI), std::move(tempUi), std::move(tempD));
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

                auto it = test.begin();
                auto cit = test.cbegin();

                for (int i = 0; i < 3; ++i, ++it, ++cit) {
                    REQUIRE(*(test.begin() + i) == i);
                    REQUIRE(*(test.cbegin() + i) == i);

                    REQUIRE(*it == uint32_t(i));
                    REQUIRE(*cit == uint32_t(i));
                }
            }
            SECTION("const accessors") {
                Pool const &ctest = test;

                REQUIRE(ctest.begin() + cTestNum == ctest.end());
                REQUIRE(ctest.cbegin() + cTestNum == ctest.cend());

                REQUIRE(ctest.begin() + cTestNum == ctest.cend());
                REQUIRE(ctest.cbegin() + cTestNum == ctest.end());

                auto it = test.begin();
                auto cit = test.cbegin();

                for (int i = 0; i < cTestNum; ++i, ++it, ++cit) {
                    REQUIRE(*(test.begin() + i) == i);
                    REQUIRE(*(test.cbegin() + i) == i);

                    REQUIRE(*it == uint32_t(i));
                    REQUIRE(*cit == uint32_t(i));
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
                    REQUIRE(*(test.begin() + i) == i);
                    REQUIRE(*(test.cbegin() + i) == i);

                    REQUIRE(*it == uint32_t(i));
                    REQUIRE(*cit == uint32_t(i));
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
                    REQUIRE(*(test.begin() + i) == i);
                    REQUIRE(*(test.cbegin() + i) == i);

                    REQUIRE(*it == uint32_t(i));
                    REQUIRE(*cit == uint32_t(i));
                }
            }
        }
    }
}

TEST_CASE("Pool<MULTI> - Testing insert/regular storage expansion") {
    // Starting the pool with essentiall no storage, and inserting more than the storage
    // initially holds to test that the expansion mechanisms work correctly.
    Pool test;

    REQUIRE(test.capacity() == 0);

    constexpr int cTestNum = 3;

    SECTION("Entity doesn't exist in pool before insertion") {
        for (int i = cTestNum - 1; i >= 0; --i) {
            REQUIRE(!test.exist(uint32_t(i)));
        }
    }

    for (int i = cTestNum - 1; i >= 0; --i) {
        int tempI = i;
        unsigned int tempUi = i;
        double tempD = i;
        test.insert(uint32_t(i), std::move(tempI), std::move(tempUi), std::move(tempD));
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

                auto it = test.begin();
                auto cit = test.cbegin();

                for (int i = 0; i < 3; ++i, ++it, ++cit) {
                    REQUIRE(*(test.begin() + i) == i);
                    REQUIRE(*(test.cbegin() + i) == i);

                    REQUIRE(*it == uint32_t(i));
                    REQUIRE(*cit == uint32_t(i));
                }
            }
            SECTION("const accessors") {
                Pool const &ctest = test;

                REQUIRE(ctest.begin() + cTestNum == ctest.end());
                REQUIRE(ctest.cbegin() + cTestNum == ctest.cend());

                REQUIRE(ctest.begin() + cTestNum == ctest.cend());
                REQUIRE(ctest.cbegin() + cTestNum == ctest.end());

                auto it = test.begin();
                auto cit = test.cbegin();

                for (int i = 0; i < cTestNum; ++i, ++it, ++cit) {
                    REQUIRE(*(test.begin() + i) == i);
                    REQUIRE(*(test.cbegin() + i) == i);

                    REQUIRE(*it == uint32_t(i));
                    REQUIRE(*cit == uint32_t(i));
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
                    REQUIRE(*(test.begin() + i) == i);
                    REQUIRE(*(test.cbegin() + i) == i);

                    REQUIRE(*it == uint32_t(i));
                    REQUIRE(*cit == uint32_t(i));
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
                    REQUIRE(*(test.begin() + i) == i);
                    REQUIRE(*(test.cbegin() + i) == i);

                    REQUIRE(*it == uint32_t(i));
                    REQUIRE(*cit == uint32_t(i));
                }
            }
        }
    }
}

TEST_CASE("Pool<MULTI> - Staggered insertion") {
    // In this case, we'll first insert a group of even number in a pass, then in the next pass,
    // the interleaving set of odd numbers, ensuring that they interleave and sort out
    // correctly.
    Pool test;
    constexpr int cTestNum = 20;

    for (int i = cTestNum; i >= 0; i -= 2) {
        int tempI = i;
        unsigned int tempUi = i;
        double tempD = i;
        test.insert(uint32_t(i), std::move(tempI), std::move(tempUi), std::move(tempD));
    }

    test.maintenance();

    SECTION("even items should exist, and in ascending order") {
        auto *it = test.begin();
        auto *dataIt = test.begin<1>();
        auto inIt = test.inbegin();

        for (int i = 0; i < cTestNum; i += 2, ++it, ++dataIt, ++inIt) {
            REQUIRE(*it == uint32_t(i));
            REQUIRE(*dataIt == i);

            REQUIRE(*(test.begin() + *inIt) == uint32_t(i));
            REQUIRE(*(test.begin() + *inIt) == i);
        }
    }
    SECTION("off numbers don't exist yet") {
        for (int i = 1; i < cTestNum; i += 2) {
            REQUIRE(!test.exist(uint32_t(i)));
        }
    }

    // Inserting the odd numbers
    for (int i = cTestNum - 1; i >= 0; i -= 2) {
        int tempI = i;
        unsigned int tempUi = i;
        double tempD = i;
        test.insert(uint32_t(i), std::move(tempI), std::move(tempUi), std::move(tempD));
    }

    test.maintenance();

    SECTION("all numbers exist, in correct order") {
        auto *it = test.begin();
        auto *dataIt = test.begin<1>();

        for (int i = 0; i < cTestNum; ++i, ++it, ++dataIt) {
            REQUIRE(*it == uint32_t(i));
            REQUIRE(*dataIt == i);
        }
    }
    SECTION("odd numbered insertions are the only ones in the insertion list, in order") {
        auto inIt = test.inbegin();

        for (int i = 1; i < cTestNum; i += 2, ++inIt) {
            REQUIRE(*(test.begin() + *inIt) == uint32_t(i));
            REQUIRE(*(test.begin() + *inIt) == i);
        }
    }
}

TEST_CASE("Pool<MULTI> - Attempting to insert multiple of the same uint32_t fails, only the "
          "first one is inserted") {
    Pool test;

    int tempI = 1;
    unsigned int tempUi = 1;
    double tempD = 1.;
    test.insert(uint32_t(16), std::move(tempI), std::move(tempUi), std::move(tempD));
    tempI = 2;
    tempUi = 2;
    tempD = 2.;
    test.insert(uint32_t(16), std::move(tempI), std::move(tempUi), std::move(tempD));
    tempI = 3;
    tempUi = 3;
    tempD = 3.;
    test.insert(uint32_t(16), std::move(tempI), std::move(tempUi), std::move(tempD));

    test.maintenance();

    REQUIRE(test.size() == 1);
    REQUIRE(test.inserted() == 1);

    REQUIRE(*test.begin() == uint32_t(16));
    REQUIRE(*test.begin<1>() == 1);
}

TEST_CASE("Pool<MULTI> - Attempting to add same entity in a different pass fails, original "
          "stays intact") {
    Pool test;

    int tempI = 1;
    unsigned int tempUi = 1;
    double tempD = 1.;
    test.insert(uint32_t(16), std::move(tempI), std::move(tempUi), std::move(tempD));
    test.maintenance();

    tempI = 2;
    tempUi = 2;
    tempD = 2.;
    test.insert(uint32_t(16), std::move(tempI), std::move(tempUi), std::move(tempD));
    test.maintenance();

    REQUIRE(test.size() == 1);
    REQUIRE(test.inserted() == 0);

    REQUIRE(*test.begin() == uint32_t(16));
    REQUIRE(*test.begin<1>() == 1);
}

/** REMOVALS **/

TEST_CASE("Pool<MULTI> - Single removal") {
    Pool test;

    int tempI = 128;
    unsigned int tempUi = 128;
    double tempD = 128.;
    test.insert(uint32_t(256), std::move(tempI), std::move(tempUi), std::move(tempD));

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

                REQUIRE(*test.rmbegin() == uint32_t(256));
                REQUIRE(*test.crmbegin() == uint32_t(256));

                REQUIRE(*test.rmbegin<1>() == 128);
                REQUIRE(*test.crmbegin<1>() == 128);
            }
        }
    }
}

TEST_CASE("Pool<MULTI> - Multiple removal") {
    Pool test;
    constexpr int cTestNum = 20;

    for (int i = 0; i < cTestNum; ++i) {
        int tempI = i;
        unsigned int tempUi = i;
        double tempD = i;
        test.insert(uint32_t(i), std::move(tempI), std::move(tempUi), std::move(tempD));
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

                auto *it = test.rmbegin();
                auto *dataIt = test.rmbegin<1>();
                auto *cit = test.crmbegin();
                auto *cDataIt = test.crmbegin<1>();

                for (int i = 0; i < cTestNum; ++i, ++it, ++dataIt, ++cit, ++cDataIt) {
                    REQUIRE(*(test.rmbegin() + i) == i);
                    REQUIRE(*(test.crmbegin() + i) == i);

                    REQUIRE(*it == uint32_t(i));
                    REQUIRE(*cit == uint32_t(i));

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

                auto *it = test.rmbegin();
                auto *dataIt = test.rmbegin<1>();
                auto *cit = test.crmbegin();
                auto *cDataIt = test.crmbegin<1>();

                for (int i = 0; i < cTestNum; ++i, ++it, ++dataIt, ++cit, ++cDataIt) {
                    REQUIRE(*(test.rmbegin() + i) == i);
                    REQUIRE(*(test.crmbegin() + i) == i);

                    REQUIRE(*it == uint32_t(i));
                    REQUIRE(*cit == uint32_t(i));

                    REQUIRE(*dataIt == i);
                    REQUIRE(*cDataIt == i);
                }
            }
        }
    }
}

TEST_CASE("Pool<MULTI> - Staggered removal") {
    // All items are added, then just the odd ones are removed, making sure that removals are
    // correct.
    Pool test;
    constexpr int cTestNum = 20;

    for (int i = 0; i < cTestNum; ++i) {
        int tempI = i;
        unsigned int tempUi = i;
        double tempD = i;
        test.insert(uint32_t(i), std::move(tempI), std::move(tempUi), std::move(tempD));
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
        auto *it = test.rmbegin();
        auto *dataIt = test.rmbegin<1>();

        for (int i = 1; i < cTestNum; i += 2, ++it, ++dataIt) {
            REQUIRE(!test.exist(uint32_t(i)));
            REQUIRE(*it == uint32_t(i));
            REQUIRE(*dataIt == i);
        }
    }
    SECTION("Entities that should remain do so, and in order") {
        auto *it = test.begin();
        auto *dataIt = test.begin<1>();

        for (int i = 0; i < cTestNum; i += 2, ++it, ++dataIt) {
            REQUIRE(test.exist(uint32_t(i)));
            REQUIRE(*it == uint32_t(i));
            REQUIRE(*dataIt == i);
        }
    }
}

TEST_CASE("Pool<MULTI> - Attempting to remove an item multiple times doesn't have "
          "undesired effects") {
    Pool test;

    int tempI = 10;
    unsigned int tempUi = 10;
    double tempD = 10.;
    test.insert(uint32_t(10), std::move(tempI), std::move(tempUi), std::move(tempD));
    tempI = 12;
    tempUi = 12;
    tempD = 12.;
    test.insert(uint32_t(12), std::move(tempI), std::move(tempUi), std::move(tempD));
    tempI = 8;
    tempUi = 8;
    tempD = 8.;
    test.insert(uint32_t(8), std::move(tempI), std::move(tempUi), std::move(tempD));

    test.maintenance();

    REQUIRE(test.size() == 3);

    test.remove(uint32_t(10));
    test.remove(uint32_t(10));

    test.maintenance();

    REQUIRE(test.size() == 2);
    REQUIRE(test.removed() == 1);

    SECTION("Remaining items") {
        REQUIRE(*test.begin() == uint32_t(8));
        REQUIRE(*test.begin() == 8);

        REQUIRE(*(test.begin() + 1) == uint32_t(12));
        REQUIRE(*(test.begin() + 1) == 12);
    }
    SECTION("Removed item") {
        REQUIRE(*test.rmbegin() == uint32_t(10));
        REQUIRE(*test.rmbegin() == 10);
    }
}

TEST_CASE("Pool<MULTI> - Attempting to remove items not in the pool has no undesired effects") {
    Pool test;

    int tempI = 1;
    unsigned int tempUi = 1;
    double tempD = 1.;
    test.insert(uint32_t(1), std::move(tempI), std::move(tempUi), std::move(tempD));
    tempI = 3;
    tempUi = 3;
    tempD = 3.;
    test.insert(uint32_t(3), std::move(tempI), std::move(tempUi), std::move(tempD));
    tempI = 5;
    tempUi = 5;
    tempD = 5.;
    test.insert(uint32_t(5), std::move(tempI), std::move(tempUi), std::move(tempD));

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

TEST_CASE("Pool<MULTI> - Search/find functionality") {
    Pool test;
    constexpr int cBuffer = 16;
    constexpr int cTestNum = 16;

    for (int i = cBuffer; i < cBuffer + cTestNum; ++i) {
        int tempI = i;
        unsigned int tempUi = i;
        double tempD = i;
        test.insert(uint32_t(i), std::move(tempI), std::move(tempUi), std::move(tempD));
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
        SECTION("Inserted items can be found") {
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