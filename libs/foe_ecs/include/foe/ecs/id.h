// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_ECS_ID_H
#define FOE_ECS_ID_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// The full ID for some object, combined of it's constituent parts
typedef uint32_t foeId;

/// An Invalid ID value is the maximum possible total value that the type can hold
#define FOE_INVALID_ID 0

/// An Invalid ID value is the maximum possible total value that the type can hold
#define FOE_MAX_ID UINT32_MAX

/// Represents an Id's 'Group' portion of an ID
typedef foeId foeIdGroup;
/// Represents the value of an ID's Group, shifted to be zero-based
typedef foeIdGroup foeIdGroupValue;
/// Represents an index within a particular 'Group'
typedef foeId foeIdIndex;
/// Represents the value of an ID's Index, shifted to be zero-based
typedef foeIdIndex foeIdIndexValue;

/// Identifier specifically for a resource type
typedef foeId foeResourceID;
/// Identifier specifically for entities/state data
typedef foeId foeEntityID;

/// Number of total bits in the foeId type
#define foeIdNumBits 32u
/// Number of bytes in the foeId type
#define foeIdNumBytes (foeIdNumBits / 8u)
/// Number of bits used for the ID's group
#define foeIdNumGroupBits 4u
/// Number of bits used for the ID's index
#define foeIdNumIndexBits (foeIdNumBits - (foeIdNumGroupBits))

// ID Group

/// Number of bits that values are shifted for the Id Group
#define foeIdGroupBitShift foeIdNumIndexBits

/// Valid GroupID bits
#define foeIdGroupBits ((FOE_MAX_ID >> (foeIdNumBits - foeIdNumGroupBits)) << foeIdGroupBitShift)
/// Maximum zero-based value of a GroupID
#define foeIdGroupMaxValue (foeIdGroupBits >> foeIdGroupBitShift)

/// Number of reserved ID groups
#define foeIdReservedGroups 2u
/// Number of non-reserved ID groups
#define foeIdNumDynamicGroups (foeIdGroupMaxValue - foeIdReservedGroups + 1u)

/// Zero-based value representing the 'Temporary' ID group
#define foeIdTemporaryGroupValue foeIdGroupMaxValue
/// Zero-based value representing the 'Persistent' ID group
#define foeIdPersistentGroupValue (foeIdGroupMaxValue - 1)
/// Zero-based value representing the maximum number of dynamic ID groups
#define foeIdMaxDynamicGroupValue (foeIdGroupMaxValue - foeIdReservedGroups)

/// Shifted enum representing the 'Persistent' ID group
#define foeIdPersistentGroup (foeIdPersistentGroupValue << foeIdGroupBitShift)
/// Shifted enum representing the 'Temporary' ID group
#define foeIdTemporaryGroup (foeIdTemporaryGroupValue << foeIdGroupBitShift)

inline foeIdGroup foeIdGetGroup(foeId id) { return (id & foeIdGroupBits); }

inline foeIdGroup foeIdValueToGroup(foeIdGroupValue groupValue) {
    return (foeIdGroup)groupValue << foeIdGroupBitShift;
}

inline foeIdGroupValue foeIdGroupToValue(foeId id) {
    return foeIdGetGroup(id) >> foeIdGroupBitShift;
}

// ID Index

/// Number of bits that values are shifted for the Id Index
#define foeIdIndexBitShift 0

/// Bitflag of the valid IndexID bits
#define foeIdIndexBits (FOE_MAX_ID >> (foeIdNumGroupBits))
/// Minimum value of an IndexID (to avoid overlapping with FOE_INVALID_ID)
#define foeIdIndexMinValue 0x1u
/// Maximum value of an IndexID
#define foeIdIndexMaxValue (foeIdIndexBits >> foeIdIndexBitShift)

/// Maximum IdIndex
#define foeIdIndexMax (foeIdIndexMaxValue << foeIdIndexBitShift)

inline foeIdIndex foeIdGetIndex(foeId id) { return (id & foeIdIndexBits); }

inline foeIdIndex foeIdValueToIndex(foeIdIndexValue indexValue) {
    return (foeIdIndex)indexValue << foeIdIndexBitShift;
}

inline foeIdIndexValue foeIdIndexToValue(foeId id) {
    return foeIdGetIndex(id) >> foeIdIndexBitShift;
}

// Other

inline foeId foeIdCreate(foeIdGroup group, foeIdIndex index) { return group | index; }

#ifdef __cplusplus
}
#endif

#endif // FOE_ECS_ID_H