// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/filesystem.hpp>

#ifdef _WIN32
#include <Shlobj.h>
#else
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#endif

auto foeGetUserHomeDirectory() -> std::filesystem::path {
#ifdef _WIN32
    char homeDir[MAX_PATH + 1];
    if (!SHGetSpecialFolderPathA(HWND_DESKTOP, homeDir, CSIDL_PROFILE, FALSE))
        homeDir[0] = '\0';
#else
    char const *homeDir = getenv("HOME");

    if (homeDir == nullptr)
        homeDir = getpwuid(getuid())->pw_dir;
#endif

    return homeDir;
}