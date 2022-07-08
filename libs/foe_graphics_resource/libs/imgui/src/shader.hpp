// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SHADER_HPP
#define SHADER_HPP

struct foeShader;
struct foeShaderCreateInfo;

void imgui_foeShader(foeShader const *pResource);

void imgui_foeShaderCreateInfo(foeShaderCreateInfo const *pCreateInfo);

#endif // SHADER_HPP