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

#ifndef FOE_ECS_ENTITY_ID_HPP
#define FOE_ECS_ENTITY_ID_HPP

#include <cstdint>
#include <limits>
#include <string>
#include <sstream>
#include <iomanip>

using foeEntityID = uint32_t;
using foeGroupID = foeEntityID;
using foeIndexID = foeEntityID;

enum : uint32_t {
    /// Number of total bits in the foeEntityID type
    foeEcsNumBits = static_cast<uint32_t>(std::numeric_limits<foeEntityID>::digits),
    /// Number of total bytes in the foeEntityID type
    foeEcsNumBytes = foeEcsNumBits / 8,
    /// Number of bits used for the foeGroupID
    foeEcsNumGroupBits = 4,
    /// Number of bits used for the foeGroupID
    foeEcsNumIndexBits = foeEcsNumBits - foeEcsNumGroupBits,
};

enum : foeEntityID {
    /// The invalid ID
    foeInvalidId = std::numeric_limits<foeEntityID>::max(),

    /// Bitflag of the valid GroupID bits
    foeEcsValidGroupBits = foeInvalidId << foeEcsNumIndexBits,
    /// Maximum value of a GroupID
    foeEcsMaxGroupValue = foeInvalidId >> foeEcsNumIndexBits,

    /// Bitflag of the valid IndexID bits
    foeEcsValidIndexBits = foeInvalidId >> foeEcsNumGroupBits,
    /// Maximum value of an IndexID
    foeEcsMaxIndexValue = foeEcsValidIndexBits - 1,
    /// The invalid index value
    foeEcsInvalidIndexID = foeEcsValidIndexBits,
};

#define FOE_INVALID_ENTITY foeInvalidId

inline foeGroupID foeEcsNormalizedToGroupID(uint32_t groupValue) {
    return (static_cast<foeGroupID>(groupValue) << foeEcsNumIndexBits);
}

inline foeGroupID foeEcsGetGroupID(foeEntityID entity) { return (entity & foeEcsValidGroupBits); }

inline uint32_t foeEcsGetNormalizedGroupID(foeEntityID entity) {
    return foeEcsGetGroupID(entity) >> foeEcsNumIndexBits;
}

inline foeGroupID foeEcsGetIndexID(foeEntityID entity) { return (entity & foeEcsValidIndexBits); }

inline std::string foeEntityID_to_string(foeEntityID entity) {
    constexpr int printWidth = foeEcsNumBytes * 2;

    std::stringstream ss;
    ss << "0x" << std::hex << std::setw(printWidth) << std::uppercase << std::setfill('0') << entity;
    return ss.str();
}

#endif // FOE_ECS_ENTITY_ID_HPP