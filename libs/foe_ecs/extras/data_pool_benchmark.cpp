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

#include <fmt/core.h>
#include <foe/chrono/easy_clock.hpp>
#include <foe/ecs/data_pool.hpp>

#include "reference_pool_map_template.hpp"
#include "test_seed.hpp"

#include <iostream>
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

int main(int argc, char **argv) {
    constexpr std::string_view cTableLayout = "{:28}{:<15}{:<15}{:<15}{:<15}{:<15}\n";
    constexpr int cTestRunCount = 8;

    foeEasyHighResClock clock;
    std::chrono::nanoseconds intMapPool{0};
    std::chrono::nanoseconds intAllocPool{0};
    std::chrono::nanoseconds customMapPool{0};
    std::chrono::nanoseconds customAllocPool{0};
    std::chrono::nanoseconds multiAllocPool{0};

    fmt::print("Benchmark for Component Data Pools\n\n");
    fmt::print(cTableLayout, "Pool Type", "map<int>", "alloc<int>", "map<Custom>", "alloc<Custom>",
               "alloc<Multi>");

    { // Insert/maintenance groups of 16
        for (int i = 0; i < cTestRunCount; ++i) {
            StateDataMapPool<int> test;

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
                if (i % 16 == 0) {
                    test.maintenance();
                }
            }
            test.maintenance();
            clock.update();

            intMapPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<int> test;

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
                if (i % 16 == 0) {
                    test.maintenance();
                }
            }
            test.maintenance();
            clock.update();

            intAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            StateDataMapPool<Custom> test;

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
                if (i % 16 == 0) {
                    test.maintenance();
                }
            }
            test.maintenance();
            clock.update();

            customMapPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<Custom> test;

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(Custom{val}));
                if (i % 16 == 0) {
                    test.maintenance();
                }
            }
            test.maintenance();
            clock.update();

            customAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<int, Custom> test;

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val), std::move(Custom{val}));
                if (i % 16 == 0) {
                    test.maintenance();
                }
            }
            test.maintenance();
            clock.update();

            multiAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        fmt::print(cTableLayout, "Insert/Maint (16)",
                   std::chrono::duration_cast<std::chrono::milliseconds>(intMapPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(intAllocPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(customMapPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(customAllocPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(multiAllocPool).count());
        intMapPool = std::chrono::nanoseconds{0};
        intAllocPool = std::chrono::nanoseconds{0};
        customMapPool = std::chrono::nanoseconds{0};
        customAllocPool = std::chrono::nanoseconds{0};
        multiAllocPool = std::chrono::nanoseconds{0};
    }

    { // Insert/maintenance groups of 512
        for (int i = 0; i < cTestRunCount; ++i) {
            StateDataMapPool<int> test;

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
                if (i % 512 == 0) {
                    test.maintenance();
                }
            }
            test.maintenance();
            clock.update();

            intMapPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<int> test;

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
                if (i % 512 == 0) {
                    test.maintenance();
                }
            }
            test.maintenance();
            clock.update();

            intAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            StateDataMapPool<Custom> test;

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
                if (i % 512 == 0) {
                    test.maintenance();
                }
            }
            test.maintenance();
            clock.update();

            customMapPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<Custom> test;

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(Custom{val}));
                if (i % 512 == 0) {
                    test.maintenance();
                }
            }
            test.maintenance();
            clock.update();

            customAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<int, Custom> test;

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val), std::move(Custom{val}));
                if (i % 512 == 0) {
                    test.maintenance();
                }
            }
            test.maintenance();
            clock.update();

            multiAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        fmt::print(cTableLayout, "Insert/Maint (512)",
                   std::chrono::duration_cast<std::chrono::milliseconds>(intMapPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(intAllocPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(customMapPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(customAllocPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(multiAllocPool).count());
        intMapPool = std::chrono::nanoseconds{0};
        intAllocPool = std::chrono::nanoseconds{0};
        customMapPool = std::chrono::nanoseconds{0};
        customAllocPool = std::chrono::nanoseconds{0};
        multiAllocPool = std::chrono::nanoseconds{0};
    }

    { // Insert all calls
        for (int i = 0; i < cTestRunCount; ++i) {
            StateDataMapPool<int> test;

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            clock.update();

            intMapPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<int> test;

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            clock.update();

            intAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            StateDataMapPool<Custom> test;

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            clock.update();

            customMapPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<Custom> test;

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            clock.update();

            customAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<int, Custom> test;

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val), std::move(Custom{val}));
            }
            clock.update();

            multiAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        fmt::print(cTableLayout, "Insert calls (all)",
                   std::chrono::duration_cast<std::chrono::milliseconds>(intMapPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(intAllocPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(customMapPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(customAllocPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(multiAllocPool).count());
        intMapPool = std::chrono::nanoseconds{0};
        intAllocPool = std::chrono::nanoseconds{0};
        customMapPool = std::chrono::nanoseconds{0};
        customAllocPool = std::chrono::nanoseconds{0};
        multiAllocPool = std::chrono::nanoseconds{0};
    }

    { // Insert all maintenance
        for (int i = 0; i < cTestRunCount; ++i) {
            StateDataMapPool<int> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            clock.update();
            test.maintenance();
            clock.update();

            intMapPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<int> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            clock.update();
            test.maintenance();
            clock.update();

            intAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            StateDataMapPool<Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            clock.update();
            test.maintenance();
            clock.update();

            customMapPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            clock.update();
            test.maintenance();
            clock.update();

            customAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<int, Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val), std::move(Custom{val}));
            }
            clock.update();
            test.maintenance();
            clock.update();

            multiAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        fmt::print(cTableLayout, "Insert Maintenance (all)",
                   std::chrono::duration_cast<std::chrono::milliseconds>(intMapPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(intAllocPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(customMapPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(customAllocPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(multiAllocPool).count());
        intMapPool = std::chrono::nanoseconds{0};
        intAllocPool = std::chrono::nanoseconds{0};
        customMapPool = std::chrono::nanoseconds{0};
        customAllocPool = std::chrono::nanoseconds{0};
        multiAllocPool = std::chrono::nanoseconds{0};
    }

    { // Iteration
        for (int i = 0; i < cTestRunCount; ++i) {
            StateDataMapPool<int> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            clock.update();
            {
                auto const endDataIt = test.end();
                for (auto dataIt = test.begin(); dataIt != endDataIt; ++dataIt) {
                    dataIt->second = (dataIt->second * 2) + 3;
                }
            }
            clock.update();

            intMapPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<int> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            clock.update();
            {
                auto *idIt = test.begin();
                auto *dataIt = test.begin<1>();
                auto const *endDataIt = test.end<1>();
                for (; dataIt != endDataIt; ++idIt, ++dataIt) {
                    *dataIt = (*dataIt * 2) + 3;
                }
            }
            clock.update();

            intAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            StateDataMapPool<Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            clock.update();
            {
                auto const endDataIt = test.end();
                for (auto dataIt = test.begin(); dataIt != endDataIt; ++dataIt) {
                    dataIt->second.variant = (std::get<0>(dataIt->second.variant) * 2) + 3;
                }
            }
            clock.update();

            customMapPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            clock.update();
            {
                auto *idIt = test.begin();
                auto *dataIt = test.begin<1>();
                auto const *endDataIt = test.end<1>();
                for (; dataIt != endDataIt; ++idIt, ++dataIt) {
                    dataIt->variant = (std::get<0>(dataIt->variant) * 2) + 3;
                }
            }
            clock.update();

            customAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<int, Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val), std::move(Custom{val}));
            }
            test.maintenance();

            clock.update();
            {
                auto *idIt = test.begin();
                auto *dataIt = test.begin<1>();
                auto const *endDataIt = test.end<1>();
                for (; dataIt != endDataIt; ++idIt, ++dataIt) {
                    *dataIt = (*dataIt * 2) + 3;
                }
            }
            clock.update();

            multiAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        fmt::print(cTableLayout, "Iteration",
                   std::chrono::duration_cast<std::chrono::milliseconds>(intMapPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(intAllocPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(customMapPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(customAllocPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(multiAllocPool).count());
        intMapPool = std::chrono::nanoseconds{0};
        intAllocPool = std::chrono::nanoseconds{0};
        customMapPool = std::chrono::nanoseconds{0};
        customAllocPool = std::chrono::nanoseconds{0};
        multiAllocPool = std::chrono::nanoseconds{0};
    }

    { // Remove/Maintenance (16)
        for (int i = 0; i < cTestRunCount; ++i) {
            StateDataMapPool<int> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                test.remove(i);
                if (i % 16 == 0) {
                    test.maintenance();
                }
            }
            clock.update();

            intMapPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<int> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                test.remove(i);
                if (i % 16 == 0) {
                    test.maintenance();
                }
            }
            clock.update();

            intAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            StateDataMapPool<Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                test.remove(i);
                if (i % 16 == 0) {
                    test.maintenance();
                }
            }
            clock.update();

            customMapPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                test.remove(i);
                if (i % 16 == 0) {
                    test.maintenance();
                }
            }
            clock.update();

            customAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<int, Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val), std::move(Custom{val}));
            }
            test.maintenance();

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                test.remove(i);
                if (i % 16 == 0) {
                    test.maintenance();
                }
            }
            clock.update();

            multiAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        fmt::print(cTableLayout, "Remove/Maint (16)",
                   std::chrono::duration_cast<std::chrono::milliseconds>(intMapPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(intAllocPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(customMapPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(customAllocPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(multiAllocPool).count());
        intMapPool = std::chrono::nanoseconds{0};
        intAllocPool = std::chrono::nanoseconds{0};
        customMapPool = std::chrono::nanoseconds{0};
        customAllocPool = std::chrono::nanoseconds{0};
        multiAllocPool = std::chrono::nanoseconds{0};
    }

    { // Remove/Maintenance (512)
        for (int i = 0; i < cTestRunCount; ++i) {
            StateDataMapPool<int> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                test.remove(i);
                if (i % 512 == 0) {
                    test.maintenance();
                }
            }
            clock.update();

            intMapPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<int> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                test.remove(i);
                if (i % 512 == 0) {
                    test.maintenance();
                }
            }
            clock.update();

            intAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            StateDataMapPool<Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                test.remove(i);
                if (i % 512 == 0) {
                    test.maintenance();
                }
            }
            clock.update();

            customMapPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                test.remove(i);
                if (i % 512 == 0) {
                    test.maintenance();
                }
            }
            clock.update();

            customAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<int, Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val), std::move(Custom{val}));
            }
            test.maintenance();

            clock.update();
            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                test.remove(i);
                if (i % 512 == 0) {
                    test.maintenance();
                }
            }
            clock.update();

            multiAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        fmt::print(cTableLayout, "Remove/Maint (512)",
                   std::chrono::duration_cast<std::chrono::milliseconds>(intMapPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(intAllocPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(customMapPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(customAllocPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(multiAllocPool).count());
        intMapPool = std::chrono::nanoseconds{0};
        intAllocPool = std::chrono::nanoseconds{0};
        customMapPool = std::chrono::nanoseconds{0};
        customAllocPool = std::chrono::nanoseconds{0};
        multiAllocPool = std::chrono::nanoseconds{0};
    }

    { // Remove Calls
        for (int i = 0; i < cTestRunCount; ++i) {
            StateDataMapPool<int> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            clock.update();
            for (auto it : cTestSeed) {
                test.remove(it);
            }
            clock.update();

            intMapPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<int> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            clock.update();
            for (auto it : cTestSeed) {
                test.remove(it);
            }
            clock.update();

            intAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            StateDataMapPool<Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            clock.update();
            for (auto it : cTestSeed) {
                test.remove(it);
            }
            clock.update();

            customMapPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            clock.update();
            for (auto it : cTestSeed) {
                test.remove(it);
            }
            clock.update();

            customAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<int, Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val), std::move(Custom{val}));
            }
            test.maintenance();

            clock.update();
            for (auto it : cTestSeed) {
                test.remove(it);
            }
            clock.update();

            multiAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        fmt::print(cTableLayout, "Remove Calls (all)",
                   std::chrono::duration_cast<std::chrono::milliseconds>(intMapPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(intAllocPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(customMapPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(customAllocPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(multiAllocPool).count());
        intMapPool = std::chrono::nanoseconds{0};
        intAllocPool = std::chrono::nanoseconds{0};
        customMapPool = std::chrono::nanoseconds{0};
        customAllocPool = std::chrono::nanoseconds{0};
        multiAllocPool = std::chrono::nanoseconds{0};
    }

    { // Remove Maintenance 1 (all)
        for (int i = 0; i < cTestRunCount; ++i) {
            StateDataMapPool<int> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            for (auto it : cTestSeed) {
                test.remove(it);
            }

            clock.update();
            test.maintenance();
            clock.update();

            intMapPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<int> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            for (auto it : cTestSeed) {
                test.remove(it);
            }

            clock.update();
            test.maintenance();
            clock.update();

            intAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            StateDataMapPool<Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            for (auto it : cTestSeed) {
                test.remove(it);
            }

            clock.update();
            test.maintenance();
            clock.update();

            customMapPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            for (auto it : cTestSeed) {
                test.remove(it);
            }

            clock.update();
            test.maintenance();
            clock.update();

            customAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<int, Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val), std::move(Custom{val}));
            }
            test.maintenance();

            for (auto it : cTestSeed) {
                test.remove(it);
            }

            clock.update();
            test.maintenance();
            clock.update();

            multiAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        fmt::print(cTableLayout, "Remove Maintenance 1 (all)",
                   std::chrono::duration_cast<std::chrono::milliseconds>(intMapPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(intAllocPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(customMapPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(customAllocPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(multiAllocPool).count());
        intMapPool = std::chrono::nanoseconds{0};
        intAllocPool = std::chrono::nanoseconds{0};
        customMapPool = std::chrono::nanoseconds{0};
        customAllocPool = std::chrono::nanoseconds{0};
        multiAllocPool = std::chrono::nanoseconds{0};
    }

    { // Remove Maintenance 2 (all)
        for (int i = 0; i < cTestRunCount; ++i) {
            StateDataMapPool<int> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            for (auto it : cTestSeed) {
                test.remove(it);
            }

            test.maintenance();
            clock.update();
            test.maintenance();
            clock.update();

            intMapPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<int> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            for (auto it : cTestSeed) {
                test.remove(it);
            }

            test.maintenance();
            clock.update();
            test.maintenance();
            clock.update();

            intAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            StateDataMapPool<Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            for (auto it : cTestSeed) {
                test.remove(it);
            }

            test.maintenance();
            clock.update();
            test.maintenance();
            clock.update();

            customMapPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val));
            }
            test.maintenance();

            for (auto it : cTestSeed) {
                test.remove(it);
            }

            test.maintenance();
            clock.update();
            test.maintenance();
            clock.update();

            customAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        for (int i = 0; i < cTestRunCount; ++i) {
            foeDataPool<int, Custom> test;

            for (size_t i = 0; i < cTestSeed.size(); ++i) {
                int val = i;
                test.insert(cTestSeed[i], std::move(val), std::move(Custom{val}));
            }
            test.maintenance();

            for (auto it : cTestSeed) {
                test.remove(it);
            }

            test.maintenance();
            clock.update();
            test.maintenance();
            clock.update();

            multiAllocPool += clock.elapsed<std::chrono::nanoseconds>();
        }

        fmt::print(cTableLayout, "Remove Maintenance 2 (all)",
                   std::chrono::duration_cast<std::chrono::milliseconds>(intMapPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(intAllocPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(customMapPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(customAllocPool).count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(multiAllocPool).count());
        intMapPool = std::chrono::nanoseconds{0};
        intAllocPool = std::chrono::nanoseconds{0};
        customMapPool = std::chrono::nanoseconds{0};
        customAllocPool = std::chrono::nanoseconds{0};
        multiAllocPool = std::chrono::nanoseconds{0};
    }

    return 0;
}