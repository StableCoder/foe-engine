// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef CAMERA_IMEX_HPP
#define CAMERA_IMEX_HPP

#include <yaml-cpp/yaml.h>

#include "camera.hpp"
#include "yaml/structs.hpp"

inline char const *yaml_camera_key() { return "camera"; }

inline auto yaml_read_Camera(YAML::Node const &node) -> foeCamera {
    foeCamera data;

    yaml_read_foeCamera("", node, data);

    return data;
}

inline auto yaml_write_Camera(foeCamera const &data) -> YAML::Node {
    YAML::Node outNode;

    yaml_write_foeCamera("", data, outNode);

    return outNode;
}

#endif // CAMERA_IMEX_HPP