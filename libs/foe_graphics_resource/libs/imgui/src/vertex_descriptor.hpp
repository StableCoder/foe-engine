// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef VERTEX_DESCRIPTOR_HPP
#define VERTEX_DESCRIPTOR_HPP

struct foeVertexDescriptor;
struct foeVertexDescriptorCreateInfo;

void imgui_foeVertexDescriptor(foeVertexDescriptor const *pResource);

void imgui_foeVertexDescriptorCreateInfo(foeVertexDescriptorCreateInfo const *pCreateInfo);

#endif // VERTEX_DESCRIPTOR_HPP