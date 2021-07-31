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

#include <foe/model/assimp/flags.hpp>

#include <assimp/postprocess.h>

#include <array>

namespace {

struct EnumPair {
    std::string_view name;
    unsigned int value;
};

// Prefix on all of the enums/flags
constexpr std::string_view cPrefix = "aiProcess_";

#define PAIR_DEF(X)                                                                                \
    { #X, X }

constexpr std::array<EnumPair, 31> cPostProcessFlags = {{
    PAIR_DEF(aiProcess_CalcTangentSpace),
    PAIR_DEF(aiProcess_JoinIdenticalVertices),
    PAIR_DEF(aiProcess_MakeLeftHanded),
    PAIR_DEF(aiProcess_Triangulate),
    PAIR_DEF(aiProcess_RemoveComponent),
    PAIR_DEF(aiProcess_GenNormals),
    PAIR_DEF(aiProcess_GenSmoothNormals),
    PAIR_DEF(aiProcess_SplitLargeMeshes),
    PAIR_DEF(aiProcess_PreTransformVertices),
    PAIR_DEF(aiProcess_LimitBoneWeights),
    PAIR_DEF(aiProcess_ValidateDataStructure),
    PAIR_DEF(aiProcess_ImproveCacheLocality),
    PAIR_DEF(aiProcess_RemoveRedundantMaterials),
    PAIR_DEF(aiProcess_FixInfacingNormals),
    PAIR_DEF(aiProcess_SortByPType),
    PAIR_DEF(aiProcess_FindDegenerates),
    PAIR_DEF(aiProcess_FindInvalidData),
    PAIR_DEF(aiProcess_GenUVCoords),
    PAIR_DEF(aiProcess_TransformUVCoords),
    PAIR_DEF(aiProcess_FindInstances),
    PAIR_DEF(aiProcess_OptimizeMeshes),
    PAIR_DEF(aiProcess_OptimizeGraph),
    PAIR_DEF(aiProcess_FlipUVs),
    PAIR_DEF(aiProcess_FlipWindingOrder),
    PAIR_DEF(aiProcess_SplitByBoneCount),
    PAIR_DEF(aiProcess_Debone),
    // Not available in v4 of assimp, only v5, so need to specify them manually
    {"aiProcess_GlobalScale", 0x8000000},
    {"aiProcess_EmbedTextures", 0x10000000},
    {"aiProcess_ForceGenNormals", 0x20000000},
    {"aiProcess_DropNormals", 0x40000000},
    {"aiProcess_GenBoundingBoxes", 0x80000000},
}};

#undef PAIR_DEF

std::string formatString(std::string str) {
    // Trim left
    std::size_t cutOffset = 0;
    for (auto c : str) {
        if (::isalnum(c))
            break;
        else
            ++cutOffset;
    }
    str = str.substr(cutOffset);

    // Trim right
    cutOffset = 0;
    for (std::size_t i = 0; i < str.size(); ++i) {
        if (::isalnum(str[i]))
            cutOffset = i + 1;
    }
    str = str.substr(0, cutOffset);

    std::for_each(str.begin(), str.end(), [](char &c) { c = ::toupper(c); });

    return str;
}

} // namespace

bool foe_model_assimp_serialize(unsigned int flags, std::string *pString) {
    // There's no 0-value in the enum, just nothing
    if (flags == 0) {
        *pString = {};
        return true;
    }

    std::string retStr;
    for (auto const &it : cPostProcessFlags) {
        if (flags == 0 && !retStr.empty()) {
            break;
        }
        if ((it.value & flags) == it.value) {
            // Found a compatible bit mask, add it
            if (!retStr.empty()) {
                retStr += " | ";
            }
            retStr += it.name.substr(cPrefix.size());
            flags = flags ^ it.value;
        }
    }

    *pString = retStr;
    return true;
}

bool foe_model_assimp_parse(std::string_view string, unsigned int *pValue) {
    if (string.empty()) {
        *pValue = 0;
        return true;
    }

    unsigned int retVal = 0;
    auto start = string.begin();
    auto end = start;
    for (; end <= string.end(); ++end) {
        if (*end == '|' || (end == string.end() && start != end)) {
            std::string token{start, end};
            token = formatString(token);
            token = formatString(std::string{cPrefix} + token);

            bool found = false;
            for (auto const &it : cPostProcessFlags) {
                std::string tempName = formatString(std::string{it.name});
                if (tempName == token) {
                    found = true;
                    retVal |= it.value;
                    break;
                }
            }

            // Could not find a matching flag for token, return failure
            if (!found)
                return false;

            start = end + 1;
        }
    }

    *pValue = retVal;
    return true;
}