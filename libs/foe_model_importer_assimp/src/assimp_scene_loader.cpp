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

#include "assimp_scene_loader.hpp"

#include "ai_glm_conversion.hpp"

namespace {

void importArmatureTree(aiNode const *pNode,
                        uint32_t offset,
                        uint32_t *pTotalOffset,
                        foeArmatureNode *pArmature) {
    pArmature[offset] = foeArmatureNode{
        .name = pNode->mName.C_Str(),
        .numChildren = pNode->mNumChildren,
        .childrenOffset = (pNode->mNumChildren != 0) ? *pTotalOffset - offset : 0,
        .transformMatrix = toGlmMat4(pNode->mTransformation),
    };

    *pTotalOffset += pNode->mNumChildren;

    for (uint32_t child = 0; child < pNode->mNumChildren; ++child) {
        importArmatureTree(pNode->mChildren[child],
                           offset + pArmature[offset].childrenOffset + child, pTotalOffset,
                           pArmature);
    }
}

uint32_t getArmatureNodeCount(aiNode const *pNode) noexcept {
    uint32_t nodes = 1;

    for (uint32_t n = 0; n < pNode->mNumChildren; ++n) {
        nodes += getArmatureNodeCount(pNode->mChildren[n]);
    }

    return nodes;
}

} // namespace

auto importSceneArmature(aiScene const *pScene) -> std::vector<foeArmatureNode> {
    std::vector<foeArmatureNode> armatureNodes(getArmatureNodeCount(pScene->mRootNode));

    uint32_t totalOffset = 1;
    importArmatureTree(pScene->mRootNode, 0, &totalOffset, armatureNodes.data());

    return armatureNodes;
}

auto importAnimation(aiAnimation *pAnimation) -> foeAnimation {
    foeAnimation animation{
        .name = pAnimation->mName.C_Str(),
        .duration = pAnimation->mDuration,
        .ticksPerSecond = pAnimation->mTicksPerSecond,
    };

    for (uint32_t c = 0; c < pAnimation->mNumChannels; ++c) {
        aiNodeAnim const *pChannel = pAnimation->mChannels[c];

        foeNodeAnimationChannel channel{
            .nodeName = pChannel->mNodeName.C_Str(),
        };

        for (uint32_t k = 0; k < pChannel->mNumPositionKeys; ++k) {
            channel.positionKeys.emplace_back(foeAnimationPositionKey{
                .time = pChannel->mPositionKeys[k].mTime,
                .value = glm::vec3{pChannel->mPositionKeys[k].mValue.x,
                                   pChannel->mPositionKeys[k].mValue.y,
                                   pChannel->mPositionKeys[k].mValue.z},
            });
        }

        for (uint32_t k = 0; k < pChannel->mNumRotationKeys; ++k) {
            channel.rotationKeys.emplace_back(foeAnimationRotationKey{
                .time = pChannel->mRotationKeys[k].mTime,
                .value = glm::quat{pChannel->mRotationKeys[k].mValue.w,
                                   pChannel->mRotationKeys[k].mValue.x,
                                   pChannel->mRotationKeys[k].mValue.y,
                                   pChannel->mRotationKeys[k].mValue.z},
            });
        }

        for (uint32_t k = 0; k < pChannel->mNumScalingKeys; ++k) {
            channel.scalingKeys.emplace_back(foeAnimationScalingKey{
                .time = pChannel->mScalingKeys[k].mTime,
                .value = glm::vec3{pChannel->mScalingKeys[k].mValue.x,
                                   pChannel->mScalingKeys[k].mValue.y,
                                   pChannel->mScalingKeys[k].mValue.z},
            });
        }

        animation.nodeChannels.emplace_back(std::move(channel));
    }

    return animation;
}