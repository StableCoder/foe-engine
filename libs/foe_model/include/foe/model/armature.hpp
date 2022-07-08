// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_MODEL_ARMATURE_HPP
#define FOE_MODEL_ARMATURE_HPP

#include <glm/glm.hpp>

#include <cstdint>
#include <string>

/// Data about an armature node and it's related children (assuming a flat array)
struct foeArmatureNode {
    /// Name of the node
    std::string name;
    /// Number of child bones
    uint32_t numChildren;
    /// Offset from this node to the start of it's set of children (from the current offset)
    uint32_t childrenOffset;
    /// Default transformation matrix for the node
    glm::mat4 transformMatrix;
};

/// Bones are offsets for 3D meshes that usually map against an arbitrary armature
struct foeMeshBone {
    /// Name of the bone, which should match to the corresponding armature and a node within it
    std::string name;
    /// Offset transformation of the bone from the armature node's transformation matrix
    glm::mat4 offsetMatrix;
};

#endif // FOE_MODEL_ARMATURE_HPP