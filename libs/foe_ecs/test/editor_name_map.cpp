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
#include <foe/ecs/editor_name_map.hpp>

TEST_CASE("EditorNameMap - Adding") {
    foeEditorNameMap map;

    SECTION("ID with empty name fails") { REQUIRE_FALSE(map.add(0, "")); }

    SECTION("Adding invalid entity fails") { REQUIRE_FALSE(map.add(FOE_INVALID_ID, "real")); }

    SECTION("Adding two of same ID but different EditorNames fails") {
        REQUIRE(map.add(0, "one"));
        REQUIRE_FALSE(map.add(0, "two"));
    }

    SECTION("Adding two different ID but same EditorNames fails") {
        REQUIRE(map.add(0, "one"));
        REQUIRE_FALSE(map.add(1, "one"));
    }
}

TEST_CASE("EditorNameMap - Finding/Searching") {
    foeEditorNameMap map;
    map.add(0, "entity0");

    SECTION("Finding existing set succeeds") {
        REQUIRE(map.find("entity0") == 0);
        REQUIRE(map.find(0) == "entity0");
    }

    SECTION("Finding non-existant set fails") {
        REQUIRE(map.find("") == FOE_INVALID_ID);
        REQUIRE(map.find("entity1") == FOE_INVALID_ID);
        REQUIRE(map.find(1) == "");
    }
}

TEST_CASE("EditorNameMap - Updating") {
    foeEditorNameMap map;
    map.add(0, "entity0");
    map.add(1, "entity1");

    SECTION("Updating with unused names succeeds") { REQUIRE(map.update(0, "entity2")); }

    SECTION("Updating set with already used name fails") {
        REQUIRE_FALSE(map.update(0, "entity1"));
    }

    SECTION("Updating items not in the map fails") { REQUIRE_FALSE(map.update(2, "blah")); }

    SECTION("Updating with blank names fails") { REQUIRE_FALSE(map.update(0, "")); }
}

TEST_CASE("EditorNameMap - Removal") {
    foeEditorNameMap map;
    map.add(0, "entity0");

    SECTION("Removing set not in the map fails") { REQUIRE_FALSE(map.remove(2)); }

    SECTION("Removing set in map succeeds") {
        REQUIRE(map.remove(0));
        SECTION("Attempting to remove set already removed fails") { REQUIRE_FALSE(map.remove(0)); }
        SECTION("Adding removed set with real name succeeds") { map.add(0, "entity123"); }
    }
}