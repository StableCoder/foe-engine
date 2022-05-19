/*
    Copyright (C) 2022 George Cave.

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

#include "../src/error_code.hpp"

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        errC = X;                                                                                  \
                                                                                                   \
        CHECK(errC.value() == X);                                                                  \
        CHECK(errC.message() == #X);                                                               \
        CHECK(std::string_view{errC.category().name()} == "foePhysicsResult");                     \
    }

TEST_CASE("foePhysicsResult - Ensure error codes return correct values and strings") {
    std::error_code errC;

    SECTION("Generic non-existant negative value") {
        errC = static_cast<foePhysicsResult>(FOE_RESULT_MIN_ENUM);

        CHECK(errC.value() == FOE_RESULT_MIN_ENUM);
        CHECK(errC.message() == "FOE_PHYSICS_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        errC = static_cast<foePhysicsResult>(FOE_RESULT_MAX_ENUM);

        CHECK(errC.value() == FOE_RESULT_MAX_ENUM);
        CHECK(errC.message() == "FOE_PHYSICS_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_SUCCESS)
    // Loaders
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_ERROR_COLLISION_SHAPE_LOADER_INITIALIZATION_FAILED)
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_ERROR_INCOMPATIBLE_CREATE_INFO)
    // Physics System
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_ERROR_MISSING_COLLISION_SHAPE_LOADER)
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_ERROR_MISSING_COLLISION_SHAPE_RESOURCES)
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_ERROR_MISSING_RIGID_BODY_COMPONENTS)
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_ERROR_MISSING_POSITION_3D_COMPONENTS)
}