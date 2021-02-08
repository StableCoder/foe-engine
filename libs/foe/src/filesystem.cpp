/*
    Copyright (C) 2021 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

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