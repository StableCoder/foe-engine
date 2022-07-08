// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

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
