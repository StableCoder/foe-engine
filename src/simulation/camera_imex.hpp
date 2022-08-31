// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef CAMERA_IMEX_HPP
#define CAMERA_IMEX_HPP

#include <foe/ecs/yaml/id.hpp>
#include <foe/yaml/parsing.hpp>
#include <yaml-cpp/yaml.h>

#include "camera.hpp"

inline char const *yaml_camera_key() { return "camera"; }

inline auto yaml_read_Camera(YAML::Node const &node) -> foeCamera {
    foeCamera data;

    yaml_read_required("fov_y", node, data.fieldOfViewY);
    yaml_read_required("near_z", node, data.nearZ);
    yaml_read_required("far_z", node, data.farZ);

    return data;
}

inline auto yaml_write_Camera(foeCamera const &data) -> YAML::Node {
    YAML::Node outNode;

    if (data.fieldOfViewY != FOE_INVALID_ID) {
        yaml_write_required("fov_y", data.fieldOfViewY, outNode);
    }
    if (data.nearZ != FOE_INVALID_ID) {
        yaml_write_required("near_z", data.nearZ, outNode);
    }
    if (data.farZ != FOE_INVALID_ID) {
        yaml_write_required("far_z", data.farZ, outNode);
    }

    return outNode;
}

#endif // CAMERA_IMEX_HPP