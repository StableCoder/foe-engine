// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_RESOURCE_ARMATURE_HPP
#define FOE_RESOURCE_ARMATURE_HPP

#include <foe/model/animation.hpp>
#include <foe/model/armature.hpp>

struct foeArmature {
    std::vector<foeArmatureNode> armature;
    std::vector<foeAnimation> animations;
};

#endif // FOE_RESOURCE_ARMATURE_HPP