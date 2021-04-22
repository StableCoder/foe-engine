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

#version 450

// Assembly Input
layout(location = 0) in vec3 inPosition;
layout(location = 2) in vec2 inUV;

#define BONES_PER_VERTEX 4
layout(location = 3) in uvec4 inBoneIndices;
layout(location = 4) in vec4 inBoneWeights;

// Uniforms
layout(set = 0, binding = 0) uniform CameraUBO { mat4 projViewMatrix; };
layout(set = 1, binding = 1) uniform ModelUBO { mat4 modelMatrix; };

#define MAX_BONES 4
layout(set = 2, binding = 2) uniform ArmatureUBO { mat4 boneMatrices[MAX_BONES]; };

// Outputs
layout(location = 0) out vec2 outUV;

out gl_PerVertex { vec4 gl_Position; };

mat4 getBoneTransforms() {
    mat4 boneTransforms = mat4(0);

    for (int i = 0; i < BONES_PER_VERTEX; ++i) {
        boneTransforms += boneMatrices[inBoneIndices[i]] * inBoneWeights[i];
    }

    return boneTransforms;
}

void main() {
    outUV = inUV;
    gl_Position = projViewMatrix * modelMatrix * getBoneTransforms() * vec4(inPosition, 1.0);
}