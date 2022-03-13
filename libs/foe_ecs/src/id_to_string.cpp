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
    ss << "0x" << std::hex << std::setw(groupWidth) << std::uppercase << std::setfill('0') << id;

    return ss.str();
}

std::string foeIdIndexToString(foeIdIndex id) {
    constexpr int indexWidth = (foeIdNumIndexBits / 4) + ((foeIdNumIndexBits % 4) ? 1 : 0);

    std::stringstream ss;
    ss << "0x" << std::hex << std::setw(indexWidth) << std::uppercase << std::setfill('0') << id;

    return ss.str();
}

std::string foeIdToSplitString(foeId id) {
    return foeIdGroupToString(foeIdGroupToValue(id)) + "-" +
           foeIdIndexToString(foeIdIndexToValue(id));
}