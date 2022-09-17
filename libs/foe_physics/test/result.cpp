// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/physics/result.h>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        foePhysicsResultToString(X, resultString);                                                 \
        CHECK(std::string_view{resultString} == #X);                                               \
    }

TEST_CASE("foePhysicsResult - Ensure error codes return correct values and strings") {
    char resultString[FOE_MAX_RESULT_STRING_SIZE];

    SECTION("Generic non-existant negative value") {
        foePhysicsResultToString((foePhysicsResult)FOE_RESULT_MIN_ENUM, resultString);
        CHECK(std::string_view{resultString} == "FOE_PHYSICS_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        foePhysicsResultToString((foePhysicsResult)FOE_RESULT_MAX_ENUM, resultString);
        CHECK(std::string_view{resultString} == "FOE_PHYSICS_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_ERROR_OUT_OF_MEMORY)
    // Loaders
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_ERROR_COLLISION_SHAPE_LOADER_INITIALIZATION_FAILED)
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_ERROR_INCOMPATIBLE_CREATE_INFO)
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_ERROR_INCOMPATIBLE_RESOURCE_TYPE)
    // Physics System
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_ERROR_MISSING_COLLISION_SHAPE_LOADER)
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_ERROR_MISSING_COLLISION_SHAPE_RESOURCES)
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_ERROR_MISSING_RIGID_BODY_COMPONENTS)
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_ERROR_MISSING_POSITION_3D_COMPONENTS)
}