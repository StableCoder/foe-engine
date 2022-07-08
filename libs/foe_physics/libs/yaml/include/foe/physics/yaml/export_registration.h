// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PHYSICS_YAML_EXPORT_REGISTRATION_H
#define FOE_PHYSICS_YAML_EXPORT_REGISTRATION_H

#include <foe/error_code.h>
#include <foe/physics/yaml/export.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_PHYSICS_YAML_EXPORT foeResult foePhysicsYamlRegisterExporters();

FOE_PHYSICS_YAML_EXPORT void foePhysicsYamlDeregisterExporters();

#ifdef __cplusplus
}
#endif

#endif // FOE_PHYSICS_YAML_EXPORT_REGISTRATION_H