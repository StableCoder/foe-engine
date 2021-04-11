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
    foeIdNumBits = static_cast<uint32_t>(std::numeric_limits<foeId>::digits),
    /// Number of total bytes in the foeId type
    foeIdNumBytes = foeIdNumBits / 8,
    /// Number of bits used for the foeIdGroup
    foeIdNumGroupBits = 4,
    /// Number of bits for determining the object type the ID represents
    foeIdNumTypeBits = 1,
    /// Number of bits used for the foeIdGroup
    foeIdNumIndexBits = foeIdNumBits - (foeIdNumGroupBits + foeIdNumTypeBits),
};

// General ID

enum : foeId {
    /// The invalid ID
    foeIdInvalid = std::numeric_limits<foeId>::max(),
};

#define FOE_INVALID_ID foeIdInvalid

inline std::string foeIdToString(foeId id) {
    constexpr int printWidth = foeIdNumBytes * 2;

    std::stringstream ss;
    ss << "0x" << std::hex << std::setw(printWidth) << std::uppercase << std::setfill('0') << id;
    return ss.str();
}

// ID Group

enum : foeIdGroup {
    /// Number of bits that values are shifted for the Id Group's bits
    foeIdGroupBitShift = foeIdNumIndexBits + foeIdNumTypeBits,

    /// Bitflag of the valid GroupID bits
    foeIdValidGroupBits = foeIdInvalid << foeIdGroupBitShift,
    /// Maximum value of a GroupID
    foeIdMaxGroupValue = foeIdValidGroupBits >> foeIdGroupBitShift,

    /// Entities that are to be preserved across sessions
    foeIdPersistentGroup = (foeIdMaxGroupValue - 1) << (foeIdNumIndexBits + foeIdNumTypeBits),
    /// Entities that are not to be preseved, and are just local to the current session
    foeIdTemporaryGroup = foeIdMaxGroupValue << (foeIdNumIndexBits + foeIdNumTypeBits),
    /// Max number possible of general groups
    foeIdMaxDynamicGroups = foeIdMaxGroupValue - 2,
};

inline foeIdGroup foeIdGetGroup(foeId id) { return (id & foeIdValidGroupBits); }

inline foeIdGroup foeIdValueToGroup(foeIdGroup groupValue) {
    return static_cast<foeIdGroup>(groupValue) << (foeIdNumIndexBits + foeIdNumTypeBits);
}

inline uint32_t foeIdGroupToValue(foeId id) {
    return foeIdGetGroup(id) >> (foeIdNumIndexBits + foeIdNumTypeBits);
}

// ID Type

enum : foeIdType {
    /// Number of bits that values are shifted for the Id Type's bits
    foeIdTypeBitShift = foeIdNumIndexBits,

    /// Bitflag of the valid Type bits
    foeIdValidTypeBits =
        (foeIdInvalid << (foeIdNumIndexBits + foeIdNumGroupBits)) >> (foeIdNumGroupBits),
    /// Maximum value that the type section can represent
    foeIdMaxTypeValue = foeIdValidTypeBits >> foeIdNumIndexBits,
};

inline bool foeIdIsResource(foeId id) { return id & foeIdValidTypeBits; }

inline bool foeIdIsEntity(foeId id) { return id ^ foeIdValidTypeBits; }

// ID Index

enum : foeIdIndex {
    /// Bitflag of the valid IndexID bits
    foeIdValidIndexBits = foeIdInvalid >> (foeIdNumGroupBits + foeIdNumTypeBits),
    /// The invalid index value
    foeIdInvalidIndex = foeIdValidIndexBits,
    /// Maximum value of an IndexID
    foeIdMaxIndexValue = foeIdValidIndexBits - 1,
};

inline foeIdGroup foeIdGetIndex(foeId id) { return (id & foeIdValidIndexBits); }

#endif // FOE_ID_HPP