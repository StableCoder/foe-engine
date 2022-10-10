// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ARMATURE_HPP
#define ARMATURE_HPP

#include <foe/model/animation.hpp>
#include <foe/model/armature.hpp>

struct foeArmature {
    std::vector<foeArmatureNode> armature;
    std::vector<foeAnimation> animations;
};

#endif // ARMATURE_HPP