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

#ifndef FOE_ID_HPP
#define FOE_ID_HPP

#include <cstdint>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

using foeId = uint32_t;
using foeIdGroup = foeId;
using foeIdType = foeId;
using foeIdIndex = foeId;

enum : uint32_t {
    /// Number of total bits in the foeId type
    foeEcsNumBits = static_cast<uint32_t>(std::numeric_limits<foeId>::digits),
    /// Number of total bytes in the foeId type
    foeEcsNumBytes = foeEcsNumBits / 8,
    /// Number of bits used for the foeIdGroup
    foeEcsNumGroupBits = 4,
    /// Number of bits for determining the object type the ID represents
    foeEcsNumTypeBits = 1,
    /// Number of bits used for the foeIdGroup
    foeEcsNumIndexBits = foeEcsNumBits - (foeEcsNumGroupBits + foeEcsNumTypeBits),
};

enum : foeId {
    /// The invalid ID
    foeInvalidId = std::numeric_limits<foeId>::max(),

    /// Bitflag of the valid GroupID bits
    foeEcsValidGroupBits = foeInvalidId << (foeEcsNumIndexBits + foeEcsNumTypeBits),
    /// Maximum value of a GroupID
    foeEcsMaxGroupValue = foeEcsValidGroupBits >> (foeEcsNumIndexBits + foeEcsNumTypeBits),

    // Bitflag of the valid Type bits
    foeEcsValidTypeBits =
        (foeInvalidId << (foeEcsNumIndexBits + foeEcsNumGroupBits)) >> (foeEcsNumGroupBits),
    foeEcsMaxTypeValue = foeEcsValidTypeBits >> foeEcsNumIndexBits,

    /// Bitflag of the valid IndexID bits
    foeEcsValidIndexBits = foeInvalidId >> (foeEcsNumGroupBits + foeEcsNumTypeBits),
    /// The invalid index value
    foeEcsInvalidIndexID = foeEcsValidIndexBits,
    /// Maximum value of an IndexID
    foeEcsMaxIndexValue = foeEcsValidIndexBits - 1,
};

#define FOE_INVALID_ID foeInvalidId

inline foeIdGroup foeEcsNormalizedToGroupID(uint32_t groupValue) {
    return static_cast<foeIdGroup>(groupValue) << (foeEcsNumIndexBits + foeEcsNumTypeBits);
}

inline foeIdGroup foeEcsGetGroupID(foeId id) { return (id & foeEcsValidGroupBits); }

inline uint32_t foeEcsGetNormalizedGroupID(foeId id) {
    return foeEcsGetGroupID(id) >> (foeEcsNumIndexBits + foeEcsNumTypeBits);
}

inline bool foeEcsIsResource(foeId id) { return id & foeEcsValidTypeBits; }

inline bool foeEcsIsEntity(foeId id) { return id ^ foeEcsValidTypeBits; }

inline foeIdGroup foeEcsGetIndexID(foeId id) { return (id & foeEcsValidIndexBits); }

inline std::string foeId_to_string(foeId id) {
    constexpr int printWidth = foeEcsNumBytes * 2;

    std::stringstream ss;
    ss << "0x" << std::hex << std::setw(printWidth) << std::uppercase << std::setfill('0') << id;
    return ss.str();
}

#endif // FOE_ID_HPP