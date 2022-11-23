// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/plugin.h>

#include "log.hpp"

#if defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h>

#include <mutex>
#elif defined(_WIN32) || defined(WIN32)
#include <Windows.h>
#endif

namespace {

#if defined(__linux__) || defined(__APPLE__)
std::mutex gUnixPluginSync{};

void *loadUnixPlugin(char const *pPluginPath) {
    std::unique_lock lock{gUnixPluginSync};

    dlerror();
    void *pModule = dlopen(pPluginPath, RTLD_LAZY);
    if (pModule == nullptr) {
        auto *pErrStr = dlerror();
        FOE_LOG(foeCore, FOE_LOG_LEVEL_ERROR, "Could not load plugin file {} due to error: {}",
                pPluginPath, pErrStr);
    }

    return pModule;
}

void *getUnixPluginSymbolAddr(void *pModule, char const *pSymbol) {
    std::unique_lock lock{gUnixPluginSync};

    dlerror();
    void *pSymbolAddr = dlsym(pModule, pSymbol);
    if (pSymbolAddr == nullptr) {
        auto *pErrStr = dlerror();
        FOE_LOG(foeCore, FOE_LOG_LEVEL_ERROR,
                "Could not find plugin symbol {} in plugin module {} due to error: {}", pSymbol,
                pModule, pErrStr);
    }

    return pSymbolAddr;
}

void unloadUnixPlugin(void *pModule) {
    // Through the definition of `DISABLE_PLUGIN_UNLOAD`, can stop plugins from being unloaded which
    // can make debugging issues easier, especially in cases such as sanitizers which analyze data
    // after the program terminates.
#ifndef DISABLE_PLUGIN_UNLOAD
    dlclose(pModule);
#endif
}
#endif

#if defined(_WIN32) || defined(WIN32)
static_assert(sizeof(foePlugin) >= sizeof(HMODULE),
              "On Windows, foePlugin handle MUST be of greater or equal size of HMODULE");

void *loadWindowsPlugin(char const *pPluginPath) {
    HMODULE module = LoadLibraryA(pPluginPath);
    if (module == NULL) {
        auto errC = GetLastError();
        FOE_LOG(foeCore, FOE_LOG_LEVEL_ERROR, "Could not load plugin {} due to error: {}",
                pPluginPath, errC);
    }

    return module;
}

void *getWindowsPluginSymbolAddr(HMODULE module, char const *pSymbol) {
    void *pSymbolAddr = GetProcAddress(module, pSymbol);
    if (pSymbolAddr == nullptr) {
        auto errC = GetLastError();
        FOE_LOG(foeCore, FOE_LOG_LEVEL_ERROR,
                "Could not find plugin symbol {} in plugin module {} due to error: {}", pSymbol,
                static_cast<void *>(module), errC);
    }

    return pSymbolAddr;
}

void unloadWindowsPlugin(HMODULE module) {
    // Through the definition of `DISABLE_PLUGIN_UNLOAD`, can stop plugins from being unloaded which
    // can make debugging issues easier, especially in cases such as sanitizers which analyze data
    // after the program terminates.
#ifndef DISABLE_PLUGIN_UNLOAD
    FreeLibrary(module);
#endif
}
#endif

} // namespace

extern "C" void foeCreatePlugin(char const *pPath, foePlugin *pPlugin) {
#if defined(__linux__) || defined(__APPLE__)
    *pPlugin = reinterpret_cast<foePlugin>(loadUnixPlugin(pPath));
#elif defined(_WIN32) || defined(WIN32)
    *pPlugin = reinterpret_cast<foePlugin>(loadWindowsPlugin(pPath));
#endif
}

extern "C" void foeDestroyPlugin(foePlugin plugin) {
#if defined(__linux__) || defined(__APPLE__)
    return unloadUnixPlugin(plugin);
#elif defined(_WIN32) || defined(WIN32)
    return unloadWindowsPlugin(reinterpret_cast<HMODULE>(plugin));
#endif
}

extern "C" void *foeGetPluginSymbol(foePlugin plugin, char const *pSymbol) {
#if defined(__linux__) || defined(__APPLE__)
    return getUnixPluginSymbolAddr(plugin, pSymbol);
#elif defined(_WIN32) || defined(WIN32)
    return getWindowsPluginSymbolAddr(reinterpret_cast<HMODULE>(plugin), pSymbol);
#endif
}