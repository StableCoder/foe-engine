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
#include <foe/wsi/mouse.hpp>

constexpr int cMaxMouseButtons = 1024;

TEST_CASE("foeMouse - Initial Clean State", "[foe][wsi]") {
    foeMouse mouse = {};

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

TEST_CASE("foeMouse - Buttons in sets", "[foe][wsi]") {
    foeMouse mouse = {};

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