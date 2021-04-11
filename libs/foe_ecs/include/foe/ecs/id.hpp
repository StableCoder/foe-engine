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

// General ID

enum : foeId {
    /// The invalid ID
    foeInvalidId = std::numeric_limits<foeId>::max(),
    foeIdInvalid = foeInvalidId,
};

#define FOE_INVALID_ID foeInvalidId

// ID Group

enum : foeIdGroup {
    /// Number of bits that values are shifted for the Id Group's bits
    foeIdGroupBitShift = foeEcsNumIndexBits + foeEcsNumTypeBits,

    /// Bitflag of the valid GroupID bits
    foeEcsValidGroupBits = foeInvalidId << foeIdGroupBitShift,
    /// Maximum value of a GroupID
    foeEcsMaxGroupValue = foeEcsValidGroupBits >> foeIdGroupBitShift,

    /// Entities that are to be preserved across sessions
    foePersistentGroup = (foeEcsMaxGroupValue - 1) << (foeEcsNumIndexBits + foeEcsNumTypeBits),
    /// Entities that are not to be preseved, and are just local to the current session
    foeTemporaryGroup = foeEcsMaxGroupValue << (foeEcsNumIndexBits + foeEcsNumTypeBits),
    /// Max number possible of general groups
    foeMaxGeneralGroups = foeEcsMaxGroupValue - 2,
};

// ID Type

enum : foeIdType {
    /// Number of bits that values are shifted for the Id Type's bits
    foeIdTypeBitShift = foeEcsNumIndexBits,

    /// Bitflag of the valid Type bits
    foeEcsValidTypeBits =
        (foeInvalidId << (foeEcsNumIndexBits + foeEcsNumGroupBits)) >> (foeEcsNumGroupBits),
    /// Maximum value that the type section can represent
    foeEcsMaxTypeValue = foeEcsValidTypeBits >> foeEcsNumIndexBits,
};

// ID Index

enum : foeIdIndex {
    /// Bitflag of the valid IndexID bits
    foeEcsValidIndexBits = foeInvalidId >> (foeEcsNumGroupBits + foeEcsNumTypeBits),
    /// The invalid index value
    foeEcsInvalidIndexID = foeEcsValidIndexBits,
    /// Maximum value of an IndexID
    foeEcsMaxIndexValue = foeEcsValidIndexBits - 1,
};

inline foeIdGroup foeEcsNormalizedToGroupID(foeIdGroup groupValue) {
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