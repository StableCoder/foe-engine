// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_MODEL_ASSIMP_FLAGS_HPP
#define FOE_MODEL_ASSIMP_FLAGS_HPP

#include <foe/model/assimp/export.h>

#include <string>
#include <string_view>

/**
 * @brief Serializes an assimp bitflag
 * @param flags Value being serialized
 * @param pString Pointer to a string that will be modified with the serialized value. Only modified
 * if true is returned.
 * @return True the value was successfully serialized. False otherwise.
 */
FOE_MODEL_ASSIMP_EXPORT
bool foe_model_assimp_serialize(unsigned int flags, std::string *pString);

/**
 * @brief Parses a assimp bitflag serialized string
 * @param string String being parsed
 * @param pValue Pointer to a value that will be modified with the parsed value. Only modified if
 * true is returned.
 * @return True the value was successfully serialized. False otherwise.
 */
FOE_MODEL_ASSIMP_EXPORT
bool foe_model_assimp_parse(std::string_view string, unsigned int *pValue);

#endif // FOE_MODEL_ASSIMP_FLAGS_HPP