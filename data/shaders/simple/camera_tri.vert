/*
    Copyright (C) 2020 George Cave.

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

/*
    This shader is the basic triangle around the origin, however it is transformed 
    by a given camera projection-view matrix.
*/

#version 450

layout (set = 0, binding = 0) uniform CameraUBO {
    mat4 projViewMatrix;
};

layout(location = 0) out vec2 outUV;

out gl_PerVertex { vec4 gl_Position; };

vec2 vertexArray[3] = {
    {0.0, -0.5},
    {0.5, 0.5},
    {-0.5, 0.5},
};

void main() {
    outUV = vertexArray[gl_VertexIndex].xy;
    gl_Position = projViewMatrix * vec4(vertexArray[gl_VertexIndex], 0.0, 1.0);
}