// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/resource/resource.h>

TEST_CASE("foeResourceLoadState - Enum to string values") {
    // Unknown
    CHECK(std::string_view{foeResourceLoadStateToString((foeResourceLoadState)-1)} == "<unknown>");

    CHECK(std::string_view{foeResourceLoadStateToString(FOE_RESOURCE_LOAD_STATE_UNLOADED)} ==
          "Unloaded");
    CHECK(std::string_view{foeResourceLoadStateToString(FOE_RESOURCE_LOAD_STATE_FAILED)} ==
          "Failed");
    CHECK(std::string_view{foeResourceLoadStateToString(FOE_RESOURCE_LOAD_STATE_LOADED)} ==
          "Loaded");
}