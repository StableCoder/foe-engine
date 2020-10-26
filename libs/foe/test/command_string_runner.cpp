/*
    Copyright (C) 2020 George Cave.

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
#include <foe/command_string_runner.hpp>

#include <atomic>
#include <chrono>
#include <string_view>
#include <thread>

namespace {

bool functionRun{false};

bool checkAndResetRun() {
    auto retVal = functionRun;
    functionRun = false;
    return retVal;
}

void testFunc(std::string_view cmdStr) { functionRun = true; }

} // namespace

TEST_CASE("foeCommandStringRunner - Checking test harness", "[foe]") {
    REQUIRE_FALSE(functionRun);
    REQUIRE_FALSE(checkAndResetRun());
    REQUIRE_FALSE(functionRun);

    testFunc("");
    REQUIRE(functionRun);
    REQUIRE(checkAndResetRun());
    REQUIRE_FALSE(functionRun);
}

TEST_CASE("foeCommandStringRunner", "[foe]") {
    foeCommandStringRunner test;

    REQUIRE_FALSE(checkAndResetRun());

    SECTION("Attempting to run an empty string fails") {
        REQUIRE_FALSE(test.runCommand(""));
        REQUIRE_FALSE(checkAndResetRun());
    }
    SECTION("Attempting to run an unregistered command fails") {
        REQUIRE_FALSE(test.runCommand("lol"));
        REQUIRE_FALSE(checkAndResetRun());
    }
    SECTION("Registering a new command succeeds") {
        REQUIRE(test.registerCommand("lol", &testFunc));
        REQUIRE_FALSE(checkAndResetRun());

        SECTION("Running the registered command succeeds") {
            REQUIRE(test.runCommand("lol"));
            REQUIRE(checkAndResetRun());

            SECTION("Deregistering the command and running the same command as before fails") {
                test.deregisterCommand("lol");
                REQUIRE_FALSE(test.runCommand("lol"));
                REQUIRE_FALSE(checkAndResetRun());
            }
        }

        SECTION("Running similar but different command fails") {
            REQUIRE_FALSE(test.runCommand("llol"));
            REQUIRE_FALSE(test.runCommand("loll "));
            REQUIRE_FALSE(test.runCommand("lool"));
            REQUIRE_FALSE(checkAndResetRun());
        }

        SECTION("Attempting to re-register the same command fails") {
            REQUIRE_FALSE(test.registerCommand("lol", &testFunc));
            REQUIRE_FALSE(checkAndResetRun());
        }
        SECTION("Registering a second, different command succeeds") {
            REQUIRE(test.registerCommand("lul", &testFunc));
            REQUIRE_FALSE(checkAndResetRun());

            SECTION("Running the two commands succeeds") {
                test.runCommand("lol");
                REQUIRE(checkAndResetRun());
                test.runCommand("lul");
                REQUIRE(checkAndResetRun());

                SECTION("Attempting to run a different command fails") {
                    REQUIRE_FALSE(test.runCommand("lulz"));
                    REQUIRE_FALSE(checkAndResetRun());
                }
            }
            SECTION("Attempting to re-register either command fails") {
                REQUIRE_FALSE(test.registerCommand("lol", &testFunc));
                REQUIRE_FALSE(test.registerCommand("lul", &testFunc));
                REQUIRE_FALSE(checkAndResetRun());
            }
        }
    }
}

TEST_CASE("foeCommandStringRunner - Running a command that starts with whitespace characters",
          "[foe]") {
    foeCommandStringRunner test;

    REQUIRE_FALSE(checkAndResetRun());

    REQUIRE(test.registerCommand("lol", &testFunc));

    REQUIRE(test.runCommand(" \t\n lol"));

    REQUIRE(checkAndResetRun());
}

namespace {

foeCommandStringRunner *pTest{nullptr};
bool tooManyCooks{false};

std::atomic_bool stopCooking{false};

void registerFunc() {
    while (!stopCooking) {
        pTest->registerCommand("lol", &testFunc);
    }
}

void multiRunFunc(std::string_view cmdStr) {
    static std::atomic_int cooks{0};
    auto currentCooks = ++cooks;
    if (currentCooks > 1)
        tooManyCooks = true;
    std::this_thread::sleep_for(std::chrono::nanoseconds(5));

    --cooks;
}

void deregisterFunc() {
    while (!stopCooking) {
        pTest->deregisterCommand("lol");
    }
}

} // namespace

TEST_CASE("foeCommandStringRunner - Multi-thread", "[foe]") {
    foeCommandStringRunner test;
    pTest = &test;
    std::vector<std::thread> threads;

    // Registering
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&]() {
            while (!stopCooking)
                test.registerCommand("lol", &multiRunFunc);
        });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    stopCooking = true;
    for (auto &it : threads)
        it.join();
    threads.clear();

    // Running
    stopCooking = false;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&]() {
            while (!stopCooking)
                test.runCommand("lol");
        });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    stopCooking = true;
    for (auto &it : threads)
        it.join();
    threads.clear();

    // Deregistering
    stopCooking = false;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&]() {
            while (!stopCooking) {
                test.deregisterCommand("lol");
            }
        });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    stopCooking = true;
    for (auto &it : threads)
        it.join();
    threads.clear();

    REQUIRE_FALSE(tooManyCooks);
}