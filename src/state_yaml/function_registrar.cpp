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

#include "function_registrar.hpp"

#include <foe/resource/yaml/armature.hpp>
#include <foe/resource/yaml/image.hpp>
#include <foe/resource/yaml/material.hpp>
#include <foe/resource/yaml/mesh.hpp>
#include <foe/resource/yaml/shader.hpp>
#include <foe/resource/yaml/vertex_descriptor.hpp>

#include "distributed_yaml_generator.hpp"

bool foeYamlCoreResourceFunctionRegistrar::registerFunctions(foeImporterGenerator *pGenerator) {
    if (auto pYamlImporter = dynamic_cast<foeDistributedYamlImporterGenerator *>(pGenerator);
        pYamlImporter) {
        pYamlImporter->addImporter("armature_v1", yaml_read_armature_definition);
        pYamlImporter->addImporter("mesh_v1", yaml_read_mesh_definition);
        pYamlImporter->addImporter("material_v1", yaml_read_material_definition);
        pYamlImporter->addImporter("vertex_descriptor_v1", yaml_read_vertex_descriptor_definition);
        pYamlImporter->addImporter("shader_v1", yaml_read_shader_definition);
        pYamlImporter->addImporter("image_v1", yaml_read_image_definition);
    }

    return true;
}

bool foeYamlCoreResourceFunctionRegistrar::deregisterFunctions(foeImporterGenerator *pGenerator) {
    if (auto pYamlImporter = dynamic_cast<foeDistributedYamlImporterGenerator *>(pGenerator);
        pYamlImporter) {
        pYamlImporter->removeImporter("armature_v1");
        pYamlImporter->removeImporter("mesh_v1");
        pYamlImporter->removeImporter("material_v1");
        pYamlImporter->removeImporter("vertex_descriptor_v1");
        pYamlImporter->removeImporter("shader_v1");
        pYamlImporter->removeImporter("image_v1");
    }

    return true;
}