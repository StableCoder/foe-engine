// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/resource/resource.h>

#include <limits>

TEST_CASE("foeResourceStateFlags - Enum to string values") {
    // Unknown
    CHECK(std::string{foeResourceStateFlagBitToString(
              std::numeric_limits<foeResourceStateFlagBits>::max())} == "<UNKNOWN>");

    CHECK(std::string{foeResourceStateFlagBitToString(FOE_RESOURCE_STATE_LOADING_BIT)} ==
          "LOADING");
    CHECK(std::string{foeResourceStateFlagBitToString(FOE_RESOURCE_STATE_FAILED_BIT)} == "FAILED");
    CHECK(std::string{foeResourceStateFlagBitToString(FOE_RESOURCE_STATE_LOADED_BIT)} == "LOADED");
}