// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/ecs/id_to_string.hpp>

#include <iomanip>
#include <sstream>

std::string foeIdToString(foeId id) {
    constexpr int fullWidth = foeIdNumBits / 4;

    std::stringstream ss;
    ss << "0x" << std::hex << std::setw(fullWidth) << std::uppercase << std::setfill('0') << id;

    return ss.str();
}

std::string foeIdGroupToString(foeIdGroup id) {
    constexpr int groupWidth = (foeIdNumGroupBits / 4) + ((foeIdNumGroupBits % 4) ? 1 : 0);

    std::stringstream ss;
    ss << "0x" << std::hex << std::setw(groupWidth) << std::uppercase << std::setfill('0')
       << foeIdGroupToValue(id);

    return ss.str();
}

std::string foeIdIndexToString(foeIdIndex id) {
    constexpr int indexWidth = (foeIdNumIndexBits / 4) + ((foeIdNumIndexBits % 4) ? 1 : 0);

    std::stringstream ss;
    ss << "0x" << std::hex << std::setw(indexWidth) << std::uppercase << std::setfill('0') << id;

    return ss.str();
}