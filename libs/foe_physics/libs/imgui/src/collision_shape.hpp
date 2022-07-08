// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef COLLISION_SHAPE_HPP
#define COLLISION_SHAPE_HPP

struct foeCollisionShape;
struct foeCollisionShapeCreateInfo;

void imgui_foeCollisionShape(foeCollisionShape const *pResource);

void imgui_foeCollisionShapeCreateInfo(foeCollisionShapeCreateInfo const *pCreateInfo);

#endif // COLLISION_SHAPE_HPP