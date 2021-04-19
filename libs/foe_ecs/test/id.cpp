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
#include <foe/ecs/id.hpp>

TEST_CASE("foeID - Creating and converting IDs", "[foe][ecs]") {
    foeIdIndex index = 12345;
    foeIdGroup group = foeIdValueToGroup(12);
    foeId test;

    SECTION("Create Generic ID") {
        test = foeIdCreate(foeIdValueToGroup(0xF), 0xFF);

        REQUIRE(foeIdGroupToValue(foeIdGetGroup(test)) == 0xF);
        REQUIRE(foeIdGetIndex(test) == 0xFF);
    }

    SECTION("Create IDs with specified type") {
        test = foeIdCreateType(foeIdValueToGroup(0xF), foeIdTypeEntity, 0xFF);

        REQUIRE(foeIdGroupToValue(foeIdGetGroup(test)) == 0xF);
        REQUIRE(foeIdGetType(test) == foeIdTypeEntity);
        REQUIRE(foeIdGetIndex(test) == 0xFF);

        test = foeIdCreateType(foeIdValueToGroup(0xF), foeIdTypeResource, 0xFF);

        REQUIRE(foeIdGroupToValue(foeIdGetGroup(test)) == 0xF);
        REQUIRE(foeIdGetType(test) == foeIdTypeResource);
        REQUIRE(foeIdGetIndex(test) == 0xFF);
    }

    SECTION("Create Entity") {
        test = foeIdCreateEntity(group, index);

        REQUIRE(foeIdGetIndex(test) == index);
        REQUIRE(foeIdGetGroup(test) == group);

        REQUIRE(foeIdIsEntity(test));
        REQUIRE(!foeIdIsResource(test));
    }
    SECTION("Create Resource") {
        test = foeIdCreateResource(group, index);

        REQUIRE(foeIdGetIndex(test) == index);
        REQUIRE(foeIdGetGroup(test) == group);

        REQUIRE(foeIdIsResource(test));
        REQUIRE(!foeIdIsEntity(test));
    }

    SECTION("Converting between types") {
        test = foeIdCreateEntity(group, index);

        REQUIRE(foeIdIsEntity(test));
        REQUIRE(!foeIdIsResource(test));

        test = foeIdConvertToResource(test);

        REQUIRE(foeIdIsResource(test));
        REQUIRE(!foeIdIsEntity(test));

        test = foeIdConvertToEntity(test);

        REQUIRE(foeIdIsEntity(test));
        REQUIRE(!foeIdIsResource(test));
    }
}