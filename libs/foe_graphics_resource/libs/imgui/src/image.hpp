// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMAGE_HPP
#define IMAGE_HPP

struct foeImage;
struct foeImageCreateInfo;

void imgui_foeImage(foeImage const *pResource);

void imgui_foeImageCreateInfo(foeImageCreateInfo const *pCreateInfo);

#endif // IMAGE_HPP