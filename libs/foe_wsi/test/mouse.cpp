// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/wsi/mouse.hpp>

constexpr int cMaxMouseButtons = 1024;

TEST_CASE("foeWsiMouse - Initial Clean State", "[foe][wsi]") {
    foeWsiMouse mouse = {};

    REQUIRE(mouse.position.x == Approx(0));
    REQUIRE(mouse.position.y == Approx(0));
    REQUIRE(mouse.oldPosition.x == Approx(0));
    REQUIRE(mouse.oldPosition.y == Approx(0));
    REQUIRE(mouse.scroll.x == Approx(0));
    REQUIRE(mouse.scroll.y == Approx(0));

    REQUIRE_FALSE(mouse.inWindow);
    REQUIRE_FALSE(mouse.oldInWindow);

    REQUIRE(mouse.pressedButtons.empty());
    REQUIRE(mouse.releasedButtons.empty());
    REQUIRE(mouse.downButtons.empty());

    for (int i = 0; i < cMaxMouseButtons; ++i) {
        REQUIRE_FALSE(mouse.buttonDown(i));
    }
}

TEST_CASE("foeWsiMouse - Buttons in sets", "[foe][wsi]") {
    foeWsiMouse mouse = {};

    mouse.pressedButtons.insert(10u);

    REQUIRE(mouse.pressedButtons.size() == 1);
    REQUIRE(mouse.buttonPressed(10u));

    mouse.releasedButtons.insert(10u);

    REQUIRE(mouse.releasedButtons.size() == 1);
    REQUIRE(mouse.buttonReleased(10u));

    mouse.downButtons.insert(10u);

    REQUIRE(mouse.downButtons.size() == 1);
    REQUIRE(mouse.buttonDown(10u));
}