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

#include <foe/ecs/export.h>

#include <cstdint>
#include <limits>
#include <string>

/// The full ID for some object, combined of it's constituent parts
using foeId = uint32_t;
/// Represents an Id's 'Group' portion of an ID
using foeIdGroup = foeId;
/// Represents the value of an ID's Group, shifted to be zero-based
using foeIdGroupValue = foeIdGroup;
/// Represents an Id's 'Type' portion of an ID
using foeIdType = foeId;
/// Represents the value of an ID's Type, shifted to be zero-based
using foeIdTypeValue = foeIdType;
/// Represents an index within a particular 'Group'
using foeIdIndex = foeId;
/// Represents the value of an ID's Index, shifted to be zero-based
using foeIdIndexValue = foeIdIndex;

enum : uint32_t {
    /// Number of total bits in the foeId type
    foeIdNumBits = static_cast<foeId>(std::numeric_limits<foeId>::digits),
    /// Number of bytes in the foeId type
    foeIdNumBytes = foeIdNumBits / 8,
    /// Number of bits used for the ID's group
    foeIdNumGroupBits = 4,
    /// Number of bits used for the ID's type
    foeIdNumTypeBits = 1,
    /// Number of bits used for the ID's index
    foeIdNumIndexBits = foeIdNumBits - (foeIdNumGroupBits + foeIdNumTypeBits),
};

// General ID

// This is the maximum numerical limit value for foeId
constexpr foeId foeIdMax = std::numeric_limits<foeId>::max();

/// An Invalid ID value is the maximum possible total value that the type can hold
#define FOE_INVALID_ID 0

/** @brief Converts the foeId to a more readable hexadecimal string
 * @param id ID to convert
 * @return The ID in a hexadecimal format
 */
FOE_ECS_EXPORT std::string foeIdToString(foeId id);

// ID Group

enum : foeIdGroup {
    /// Number of bits that values are shifted for the Id Group
    foeIdGroupBitShift = foeIdNumIndexBits + foeIdNumTypeBits,

    /// Valid GroupID bits
    foeIdGroupBits = (foeIdMax >> (foeIdNumBits - foeIdNumGroupBits)) << foeIdGroupBitShift,
    /// Maximum zero-based value of a GroupID
    foeIdGroupMaxValue = foeIdGroupBits >> foeIdGroupBitShift,

    /// Number of reserved ID groups
    foeIdReservedGroups = 2,
    /// Number of non-reserved ID groups
    foeIdNumDynamicGroups = foeIdGroupMaxValue - foeIdReservedGroups + 1,

    /// Zero-based value representing the 'Temporary' ID group
    foeIdTemporaryGroupValue = foeIdGroupMaxValue,
    /// Zero-based value representing the 'Persistent' ID group
    foeIdPersistentGroupValue = (foeIdGroupMaxValue - 1),
    /// Zero-based value representing the maximum number of dynamic ID groups
    foeIdMaxDynamicGroupValue = foeIdGroupMaxValue - foeIdReservedGroups,

    /// Shifted enum representing the 'Persistent' ID group
    foeIdPersistentGroup = foeIdPersistentGroupValue << foeIdGroupBitShift,
    /// Shifted enum representing the 'Temporary' ID group
    foeIdTemporaryGroup = foeIdTemporaryGroupValue << foeIdGroupBitShift,
};

inline foeIdGroup foeIdGetGroup(foeId id) { return (id & foeIdGroupBits); }

inline foeIdGroup foeIdValueToGroup(foeIdGroupValue groupValue) {
    return static_cast<foeIdGroup>(groupValue) << foeIdGroupBitShift;
}

inline foeIdGroupValue foeIdGroupToValue(foeId id) {
    return foeIdGetGroup(id) >> foeIdGroupBitShift;
}

// ID Type

enum : foeIdType {
    /// Number of bits that values are shifted for the Id Type
    foeIdTypeBitShift = foeIdNumIndexBits,

    /// Valid Type bits
    foeIdTypeBits = (foeIdMax >> (foeIdNumBits - foeIdNumTypeBits)) << foeIdTypeBitShift,
    /// Maximum value that the Type section can represent
    foeIdTypeMaxValue = foeIdTypeBits >> foeIdNumIndexBits,

    /// Zero-based value representing a Entity-type ID
    foeIdTypeEntityValue = 0,
    /// Zero-based value representing a Resource-type ID
    foeIdTypeResourceValue = 1,

    /// Shifted enum representing a Entity-type ID
    foeIdTypeEntity = foeIdTypeEntityValue << foeIdTypeBitShift,
    /// Shifted enum representing a Resource-type ID
    foeIdTypeResource = foeIdTypeResourceValue << foeIdTypeBitShift,
};

inline foeIdType foeIdGetType(foeId id) { return id & foeIdTypeBits; }

inline foeIdType foeIdValueToType(foeIdTypeValue typeValue) {
    return static_cast<foeIdType>(typeValue) << foeIdTypeBitShift;
}

inline foeIdTypeValue foeIdTypeToValue(foeId id) { return foeIdGetType(id) >> foeIdTypeBitShift; }

inline bool foeIdIsEntity(foeId id) { return (id & foeIdTypeBits) == foeIdTypeEntity; }
inline bool foeIdIsResource(foeId id) { return (id & foeIdTypeBits) == foeIdTypeResource; }

// ID Index

enum : foeIdIndex {
    /// Number of bits that values are shifted for the Id Index
    foeIdIndexBitShift = 0,

    /// Bitflag of the valid IndexID bits
    foeIdIndexBits = foeIdMax >> (foeIdNumGroupBits + foeIdNumTypeBits),
    /// Maximum value of an IndexID
    foeIdIndexMaxValue = foeIdIndexBits >> foeIdIndexBitShift,

    /// Maximum IdIndex
    foeIdIndexMax = foeIdIndexMaxValue << foeIdIndexBitShift,
};

inline foeIdIndex foeIdGetIndex(foeId id) { return (id & foeIdIndexBits); }

inline foeIdIndex foeIdValueToIndex(foeIdIndexValue indexValue) {
    return static_cast<foeIdIndex>(indexValue) << foeIdIndexBitShift;
}

inline foeIdIndexValue foeIdIndexToValue(foeId id) {
    return foeIdGetIndex(id) >> foeIdIndexBitShift;
}

// Other ID Functions

/** @brief Converts the foeId to a more readable hexadecimal string, witht he different parts split
 * @param id ID to convert
 * @return The ID in a split-hexadecimal format
 *
 * Each of the constituent parts of the ID are split via '-', with the format being
 * Group-Type-Index.
 */
FOE_ECS_EXPORT std::string foeIdToSplitString(foeId id);

inline foeId foeIdCreate(foeIdGroup group, foeIdIndex index) { return group | index; }

inline foeId foeIdCreateType(foeIdGroup group, foeIdType type, foeIdIndex index) {
    return group | type | index;
}

inline foeId foeIdConvertToEntity(foeId id) {
    return foeIdGetGroup(id) | foeIdTypeEntity | foeIdGetIndex(id);
}

inline foeId foeIdCreateEntity(foeIdGroup group, foeIdIndex index) {
    return group | foeIdTypeEntity | index;
}

inline foeId foeIdConvertToResource(foeId id) {
    return foeIdGetGroup(id) | foeIdTypeResource | foeIdGetIndex(id);
}

inline foeId foeIdCreateResource(foeIdGroup group, foeIdIndex index) {
    return group | foeIdTypeResource | index;
}

#endif // FOE_ID_HPP