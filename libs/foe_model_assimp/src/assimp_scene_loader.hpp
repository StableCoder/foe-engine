// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ASSIMP_SCENE_LOADER_HPP
#define ASSIMP_SCENE_LOADER_HPP

#include <assimp/scene.h>
#include <foe/model/animation.hpp>
#include <foe/model/armature.hpp>

#include <vector>

auto importSceneArmature(aiScene const *pScene) -> std::vector<foeArmatureNode>;

auto importAnimation(aiAnimation *pAnimation) -> foeAnimation;

#endif // ASSIMP_SCENE_LOADER_HPP