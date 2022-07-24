// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_YAML_IMPORT_REGISTRATION_HPP
#define FOE_GRAPHICS_RESOURCE_YAML_IMPORT_REGISTRATION_HPP

#include <foe/error_code.h>
#include <foe/graphics/resource/yaml/export.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_GFX_RES_YAML_EXPORT foeResultSet foeGraphicsResourceYamlRegisterImporters();

FOE_GFX_RES_YAML_EXPORT void foeGraphicsResourceYamlDeregisterImporters();

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_RESOURCE_YAML_IMPORT_REGISTRATION_HPP