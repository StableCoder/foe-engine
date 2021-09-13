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
#include <foe/wsi/keyboard.hpp>

constexpr uint32_t cMaxKeyboardKeys = 4096;

TEST_CASE("foeWsiKeyboard - Initial Clean State", "[foe][wsi]") {
    foeWsiKeyboard keyboard = {};

    REQUIRE(keyboard.pressedKeys.empty());
    REQUIRE(keyboard.releasedKeys.empty());
    REQUIRE(keyboard.downKeys.empty());

    for (uint32_t i = 0; i < cMaxKeyboardKeys; ++i) {
        REQUIRE_FALSE(keyboard.keyPressed(i));
        REQUIRE_FALSE(keyboard.keyReleased(i));
        REQUIRE_FALSE(keyboard.keyDown(i));
    }
}

TEST_CASE("foeWsiKeyboard - Keys in sets", "[foe][wsi]") {
    foeWsiKeyboard keyboard = {};

    keyboard.pressedKeys.insert(10u);

    REQUIRE(keyboard.pressedKeys.size() == 1);
    REQUIRE(keyboard.keyPressed(10u));

    keyboard.releasedKeys.insert(10u);

    REQUIRE(keyboard.releasedKeys.size() == 1);
    REQUIRE(keyboard.keyReleased(10u));

    keyboard.downKeys.insert(10u);

    REQUIRE(keyboard.downKeys.size() == 1);
    REQUIRE(keyboard.keyDown(10u));

    keyboard.unicodeChar = 10u;
}
