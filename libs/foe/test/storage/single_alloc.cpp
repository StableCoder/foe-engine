// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/storage/single_alloc.hpp>

#include <optional>
#include <string>
#include <variant>

TEST_CASE("foeSingleAllocStorage Constructor") {
    SECTION("Default constructors have zero capacity, and all pointers are null") {
        SECTION("Single Template Argument") {
            SECTION("int") {
                foeSingleAllocStorage<int> test;

                CHECK(test.capacity() == 0);
                // First template element
                CHECK(test.get() == nullptr);
                CHECK(test.get<0>() == nullptr);
                CHECK(test.get() == test.get<0>());
            }
            SECTION("double") {
                foeSingleAllocStorage<double> test;

                CHECK(test.capacity() == 0);
                // First template element
                CHECK(test.get() == nullptr);
                CHECK(test.get<0>() == nullptr);
                CHECK(test.get() == test.get<0>());
            }
            SECTION("std::string") {
                foeSingleAllocStorage<std::string> test;

                CHECK(test.capacity() == 0);
                // First template element
                CHECK(test.get() == nullptr);
                CHECK(test.get<0>() == nullptr);
                CHECK(test.get() == test.get<0>());
            }
            SECTION("std::optional<std::string>") {
                foeSingleAllocStorage<std::optional<std::string>> test;

                CHECK(test.capacity() == 0);
                // First template element
                CHECK(test.get() == nullptr);
                CHECK(test.get<0>() == nullptr);
                CHECK(test.get() == test.get<0>());
            }
            SECTION("std::variant<int, double, std::string>") {
                foeSingleAllocStorage<std::variant<int, double, std::string>> test;

                CHECK(test.capacity() == 0);
                // First template element
                CHECK(test.get() == nullptr);
                CHECK(test.get<0>() == nullptr);
                CHECK(test.get() == test.get<0>());
            }
        }
        SECTION("Multi Template Argument") {
            SECTION("int, int, double") {
                foeSingleAllocStorage<int, int, double> test;

                CHECK(test.capacity() == 0);
                // First template element
                CHECK(test.get() == nullptr);
                CHECK(test.get<0>() == nullptr);
                CHECK(test.get() == test.get<0>());
                // Rest
                CHECK(test.get<1>() == nullptr);
                CHECK(test.get<2>() == nullptr);
            }
            SECTION("int, double, std::string, std::optional<std::string>, "
                    "std::variant<std::string, int, double>") {
                foeSingleAllocStorage<int, double, std::string, std::optional<std::string>,
                                      std::variant<std::string, int, double>>
                    test;

                CHECK(test.capacity() == 0);
                // First template element
                CHECK(test.get() == nullptr);
                CHECK(test.get<0>() == nullptr);
                CHECK(test.get() == test.get<0>());
                // Rest
                CHECK(test.get<1>() == nullptr);
                CHECK(test.get<2>() == nullptr);
                CHECK(test.get<3>() == nullptr);
                CHECK(test.get<4>() == nullptr);
            }
        }
    }
    SECTION("Constructors with a non-zero value have allocated pointers") {
        SECTION("Single Template Argument") {
            SECTION("int") {
                foeSingleAllocStorage<int> test{1};

                CHECK(test.capacity() == 1);
                // First template element
                CHECK(test.get() != nullptr);
                CHECK(test.get<0>() != nullptr);
                CHECK(test.get() == test.get<0>());
            }
            SECTION("double") {
                foeSingleAllocStorage<double> test{1};

                CHECK(test.capacity() == 1);
                // First template element
                CHECK(test.get() != nullptr);
                CHECK(test.get<0>() != nullptr);
                CHECK(test.get() == test.get<0>());
            }
            SECTION("std::string") {
                foeSingleAllocStorage<std::string> test{1};

                CHECK(test.capacity() == 1);
                // First template element
                CHECK(test.get() != nullptr);
                CHECK(test.get<0>() != nullptr);
                CHECK(test.get() == test.get<0>());
            }
            SECTION("std::optional<std::string>") {
                foeSingleAllocStorage<std::optional<std::string>> test{1};

                CHECK(test.capacity() == 1);
                // First template element
                CHECK(test.get() != nullptr);
                CHECK(test.get<0>() != nullptr);
                CHECK(test.get() == test.get<0>());
            }
            SECTION("std::variant<int, double, std::string>") {
                foeSingleAllocStorage<std::variant<int, double, std::string>> test{1};

                CHECK(test.capacity() == 1);
                // First template element
                CHECK(test.get() != nullptr);
                CHECK(test.get<0>() != nullptr);
                CHECK(test.get() == test.get<0>());
            }
        }
        SECTION("Multi Template Argument") {
            SECTION("int, int, double") {
                foeSingleAllocStorage<int, int, double> test{1};

                CHECK(test.capacity() == 1);
                // First template element
                CHECK(test.get() != nullptr);
                CHECK(test.get<0>() != nullptr);
                CHECK(test.get() == test.get<0>());
                // Rest
                CHECK(test.get<1>() != nullptr);
                CHECK(test.get<2>() != nullptr);
            }
            SECTION("int, double, std::string, std::optional<std::string>, "
                    "std::variant<std::string, int, double>") {
                foeSingleAllocStorage<int, double, std::string, std::optional<std::string>,
                                      std::variant<std::string, int, double>>
                    test{1};

                CHECK(test.capacity() == 1);
                // First template element
                CHECK(test.get() != nullptr);
                CHECK(test.get<0>() != nullptr);
                CHECK(test.get() == test.get<0>());
                // Rest
                CHECK(test.get<1>() != nullptr);
                CHECK(test.get<2>() != nullptr);
                CHECK(test.get<3>() != nullptr);
                CHECK(test.get<4>() != nullptr);
            }
        }
    }
}

TEST_CASE("foeSingleAllocStorage Move Constructor") {
    SECTION("Single Template Argument") {
        SECTION("int") {
            foeSingleAllocStorage<int> test{1};
            auto ptr0 = test.get<0>();
            foeSingleAllocStorage<int> moveTo{std::move(test)};

            SECTION("Pointers from moved-to object the same as previously") {
                CHECK(moveTo.capacity() == 1);
                CHECK(moveTo.get<0>() == ptr0);
            }
            SECTION("Original moved-from object no longer points to anything") {
                CHECK(test.capacity() == 0);
                CHECK(test.get<0>() == nullptr);
            }
        }
        SECTION("double") {
            foeSingleAllocStorage<double> test{1};
            auto ptr0 = test.get<0>();
            foeSingleAllocStorage<double> moveTo{std::move(test)};

            SECTION("Pointers from moved-to object the same as previously") {
                CHECK(moveTo.capacity() == 1);
                CHECK(moveTo.get<0>() == ptr0);
            }
            SECTION("Original moved-from object no longer points to anything") {
                CHECK(test.capacity() == 0);
                CHECK(test.get<0>() == nullptr);
            }
        }
        SECTION("std::string") {
            foeSingleAllocStorage<std::string> test{1};
            auto ptr0 = test.get<0>();
            foeSingleAllocStorage<std::string> moveTo{std::move(test)};

            SECTION("Pointers from moved-to object the same as previously") {
                CHECK(moveTo.capacity() == 1);
                CHECK(moveTo.get<0>() == ptr0);
            }
            SECTION("Original moved-from object no longer points to anything") {
                CHECK(test.capacity() == 0);
                CHECK(test.get<0>() == nullptr);
            }
        }
        SECTION("std::optional<std::string>") {
            foeSingleAllocStorage<std::optional<std::string>> test{1};
            auto ptr0 = test.get<0>();
            foeSingleAllocStorage<std::optional<std::string>> moveTo{std::move(test)};

            SECTION("Pointers from moved-to object the same as previously") {
                CHECK(moveTo.capacity() == 1);
                CHECK(moveTo.get<0>() == ptr0);
            }
            SECTION("Original moved-from object no longer points to anything") {
                CHECK(test.capacity() == 0);
                CHECK(test.get<0>() == nullptr);
            }
        }
        SECTION("std::variant<int, double, std::string>") {
            foeSingleAllocStorage<std::variant<int, double, std::string>> test{1};
            auto ptr0 = test.get<0>();
            foeSingleAllocStorage<std::variant<int, double, std::string>> moveTo{std::move(test)};

            SECTION("Pointers from moved-to object the same as previously") {
                CHECK(moveTo.capacity() == 1);
                CHECK(moveTo.get<0>() == ptr0);
            }
            SECTION("Original moved-from object no longer points to anything") {
                CHECK(test.capacity() == 0);
                CHECK(test.get<0>() == nullptr);
            }
        }
    }
    SECTION("Multi Template Argument") {
        SECTION("int, int, double") {
            foeSingleAllocStorage<int, int, double> test{1};
            auto ptr0 = test.get<0>();
            auto ptr1 = test.get<1>();
            auto ptr2 = test.get<2>();
            foeSingleAllocStorage<int, int, double> moveTo{std::move(test)};

            SECTION("Pointers from moved-to object the same as previously") {
                CHECK(moveTo.capacity() == 1);
                CHECK(moveTo.get<0>() == ptr0);
                CHECK(moveTo.get<1>() == ptr1);
                CHECK(moveTo.get<2>() == ptr2);
            }
            SECTION("Original moved-from object no longer points to anything") {
                CHECK(test.capacity() == 0);
                CHECK(test.get<0>() == nullptr);
                CHECK(test.get<1>() == nullptr);
                CHECK(test.get<2>() == nullptr);
            }
        }
        SECTION("int, double, std::string, std::optional<std::string>, "
                "std::variant<std::string, int, double>") {
            foeSingleAllocStorage<int, double, std::string, std::optional<std::string>,
                                  std::variant<std::string, int, double>>
                test{1};

            auto ptr0 = test.get<0>();
            auto ptr1 = test.get<1>();
            auto ptr2 = test.get<2>();
            auto ptr3 = test.get<3>();
            auto ptr4 = test.get<4>();
            foeSingleAllocStorage<int, double, std::string, std::optional<std::string>,
                                  std::variant<std::string, int, double>>
                moveTo{std::move(test)};

            SECTION("Pointers from moved-to object the same as previously") {
                CHECK(moveTo.capacity() == 1);
                CHECK(moveTo.get<0>() == ptr0);
                CHECK(moveTo.get<1>() == ptr1);
                CHECK(moveTo.get<2>() == ptr2);
                CHECK(moveTo.get<3>() == ptr3);
                CHECK(moveTo.get<4>() == ptr4);
            }
            SECTION("Original moved-from object no longer points to anything") {
                CHECK(test.capacity() == 0);
                CHECK(test.get<0>() == nullptr);
                CHECK(test.get<1>() == nullptr);
                CHECK(test.get<2>() == nullptr);
                CHECK(test.get<3>() == nullptr);
                CHECK(test.get<4>() == nullptr);
            }
        }
    }
}

TEST_CASE("foeSingleAllocStorage Move Operator=") {
    SECTION("Single Template Argument") {
        SECTION("int") {
            foeSingleAllocStorage<int> test{1};
            auto ptr0 = test.get<0>();
            foeSingleAllocStorage<int> moveTo = std::move(test);

            SECTION("Pointers from moved-to object the same as previously") {
                CHECK(moveTo.capacity() == 1);
                CHECK(moveTo.get<0>() == ptr0);
            }
            SECTION("Original moved-from object no longer points to anything") {
                CHECK(test.capacity() == 0);
                CHECK(test.get<0>() == nullptr);
            }
        }
        SECTION("double") {
            foeSingleAllocStorage<double> test{1};
            auto ptr0 = test.get<0>();
            foeSingleAllocStorage<double> moveTo = std::move(test);

            SECTION("Pointers from moved-to object the same as previously") {
                CHECK(moveTo.capacity() == 1);
                CHECK(moveTo.get<0>() == ptr0);
            }
            SECTION("Original moved-from object no longer points to anything") {
                CHECK(test.capacity() == 0);
                CHECK(test.get<0>() == nullptr);
            }
        }
        SECTION("std::string") {
            foeSingleAllocStorage<std::string> test{1};
            auto ptr0 = test.get<0>();
            foeSingleAllocStorage<std::string> moveTo = std::move(test);

            SECTION("Pointers from moved-to object the same as previously") {
                CHECK(moveTo.capacity() == 1);
                CHECK(moveTo.get<0>() == ptr0);
            }
            SECTION("Original moved-from object no longer points to anything") {
                CHECK(test.capacity() == 0);
                CHECK(test.get<0>() == nullptr);
            }
        }
        SECTION("std::optional<std::string>") {
            foeSingleAllocStorage<std::optional<std::string>> test{1};
            auto ptr0 = test.get<0>();
            foeSingleAllocStorage<std::optional<std::string>> moveTo = std::move(test);

            SECTION("Pointers from moved-to object the same as previously") {
                CHECK(moveTo.capacity() == 1);
                CHECK(moveTo.get<0>() == ptr0);
            }
            SECTION("Original moved-from object no longer points to anything") {
                CHECK(test.capacity() == 0);
                CHECK(test.get<0>() == nullptr);
            }
        }
        SECTION("std::variant<int, double, std::string>") {
            foeSingleAllocStorage<std::variant<int, double, std::string>> test{1};
            auto ptr0 = test.get<0>();
            foeSingleAllocStorage<std::variant<int, double, std::string>> moveTo = std::move(test);

            SECTION("Pointers from moved-to object the same as previously") {
                CHECK(moveTo.capacity() == 1);
                CHECK(moveTo.get<0>() == ptr0);
            }
            SECTION("Original moved-from object no longer points to anything") {
                CHECK(test.capacity() == 0);
                CHECK(test.get<0>() == nullptr);
            }
        }
    }
    SECTION("Multi Template Argument") {
        SECTION("int, int, double") {
            foeSingleAllocStorage<int, int, double> test{1};
            auto ptr0 = test.get<0>();
            auto ptr1 = test.get<1>();
            auto ptr2 = test.get<2>();
            foeSingleAllocStorage<int, int, double> moveTo = std::move(test);

            SECTION("Pointers from moved-to object the same as previously") {
                CHECK(moveTo.capacity() == 1);
                CHECK(moveTo.get<0>() == ptr0);
                CHECK(moveTo.get<1>() == ptr1);
                CHECK(moveTo.get<2>() == ptr2);
            }
            SECTION("Original moved-from object no longer points to anything") {
                CHECK(test.capacity() == 0);
                CHECK(test.get<0>() == nullptr);
                CHECK(test.get<1>() == nullptr);
                CHECK(test.get<2>() == nullptr);
            }
        }
        SECTION("int, double, std::string, std::optional<std::string>, "
                "std::variant<std::string, int, double>") {
            foeSingleAllocStorage<int, double, std::string, std::optional<std::string>,
                                  std::variant<std::string, int, double>>
                test{1};

            auto ptr0 = test.get<0>();
            auto ptr1 = test.get<1>();
            auto ptr2 = test.get<2>();
            auto ptr3 = test.get<3>();
            auto ptr4 = test.get<4>();
            foeSingleAllocStorage<int, double, std::string, std::optional<std::string>,
                                  std::variant<std::string, int, double>>
                moveTo = std::move(test);

            SECTION("Pointers from moved-to object the same as previously") {
                CHECK(moveTo.capacity() == 1);
                CHECK(moveTo.get<0>() == ptr0);
                CHECK(moveTo.get<1>() == ptr1);
                CHECK(moveTo.get<2>() == ptr2);
                CHECK(moveTo.get<3>() == ptr3);
                CHECK(moveTo.get<4>() == ptr4);
            }
            SECTION("Original moved-from object no longer points to anything") {
                CHECK(test.capacity() == 0);
                CHECK(test.get<0>() == nullptr);
                CHECK(test.get<1>() == nullptr);
                CHECK(test.get<2>() == nullptr);
                CHECK(test.get<3>() == nullptr);
                CHECK(test.get<4>() == nullptr);
            }
        }
    }
}