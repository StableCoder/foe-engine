/*
    Copyright (C) 2021 George Cave - gcave@stablecoder.ca

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

layout(set = 7, binding = 1) uniform sampler2D sampledImage;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outFragColour;

void main() {
    vec4 colour = texture(sampledImage, inUV);

    outFragColour = vec4(colour.rgb, 1.0);
}