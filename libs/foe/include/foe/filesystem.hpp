// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_FILESYSTEM_HPP
#define FOE_FILESYSTEM_HPP

#include <foe/export.h>

#include <filesystem>

/**
 * @brief Returns the path to the users home directory
 * @return Home directory path
 * @note Only Unix, Windows implementations
 */
FOE_EXPORT auto foeGetUserHomeDirectory() -> std::filesystem::path;

#endif // FOE_FILESYSTEM_HPP