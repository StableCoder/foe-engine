// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PHYSICS_YAML_IMPORT_REGISTRATION_H
#define FOE_PHYSICS_YAML_IMPORT_REGISTRATION_H

#include <foe/physics/yaml/export.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_PHYSICS_YAML_EXPORT foeResultSet foePhysicsYamlRegisterImporters();

FOE_PHYSICS_YAML_EXPORT void foePhysicsYamlDeregisterImporters();

#ifdef __cplusplus
}
#endif

#endif // FOE_PHYSICS_YAML_IMPORT_REGISTRATION_H