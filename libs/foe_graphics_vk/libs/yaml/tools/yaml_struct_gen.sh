#!/usr/bin/env sh
set -e

if [ "$1" == "" ]; then
    echo "ERROR: Input file not specified!"
    exit 1
fi

if [[ "$2" != "" ]]; then
    # We've been given an output file, use it
    OUT_FILE=$2
else
    # No Output file, derive from the input file
    OUT_FILE=$1_yaml.cpp
fi

printf "/*
    Copyright (C) $(date +"%Y") George Cave.

    Licensed under the Apache License, Version 2.0 (the \"License\");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an \"AS IS\" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <foe/graphics/vk/yaml/vk_type_parsing.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>
#include <vk_struct_cleanup.hpp>
#include <vulkan/vulkan.h>

#include <cstring>
#include <string>

" >$OUT_FILE

READ_REQUIRED=
READ_OPTIONAL=
WRITE_REQUIRED=
WRITE_OPTIONAL=
PTR_MEMBERS=

while read LINE; do
    if [[ $LINE = *"struct"* ]]; then
        # Start of a new struct
        STRUCT="$(cut -d ' ' -f3 <<<"$LINE")"
        PTR_MEMBERS=

        READ_REQUIRED="template <>
FOE_GFX_VK_YAML_EXPORT void yaml_read_required<$STRUCT>(std::string const &nodeName, YAML::Node const &node, $STRUCT &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if(!subNode) {
        throw foeYamlException(nodeName + \" - Required node not found to parse as '$STRUCT'\");
    }

    $STRUCT newData{};
    bool read = false;
    try {"

        READ_OPTIONAL="template <>
FOE_GFX_VK_YAML_EXPORT bool yaml_read_optional<$STRUCT>(std::string const &nodeName, YAML::Node const &node, $STRUCT &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if(!subNode) {
        return false;
    }

    $STRUCT newData{};
    bool read = false;
    try {"

        WRITE_REQUIRED="template <>
FOE_GFX_VK_YAML_EXPORT void yaml_write_required<$STRUCT>(std::string const& nodeName, $STRUCT const &data, YAML::Node &node) {
    YAML::Node writeNode;

    try {"

        WRITE_OPTIONAL="template <>
FOE_GFX_VK_YAML_EXPORT bool yaml_write_optional<$STRUCT>(std::string const& nodeName, $STRUCT const& defaultData, $STRUCT const &data, YAML::Node &node) {
    YAML::Node writeNode;
    bool addedNode = false;

    try {"

    elif [[ $LINE = *'}'* ]]; then
        # Complete the struct
        READ_REQUIRED="$READ_REQUIRED
    } catch (foeYamlException const& e) {
        vk_struct_cleanup(&newData);
        throw foeYamlException(nodeName + \"::\" + e.what());
    }

    data = newData;
}
"

        READ_OPTIONAL="$READ_OPTIONAL
    } catch (foeYamlException const& e) {
        vk_struct_cleanup(&newData);
        throw foeYamlException(nodeName + \"::\" + e.what());
    }

    if(read)
        data = newData;
    return read;
}
"

        WRITE_REQUIRED="$WRITE_REQUIRED
    } catch (foeYamlException const& e) {
        throw foeYamlException(nodeName + \"::\" + e.what());
    }

    if(nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}
"

        WRITE_OPTIONAL="$WRITE_OPTIONAL
    } catch (foeYamlException const& e) {
        throw foeYamlException(nodeName + \"::\" + e.what());
    }

    if(addedNode) {
        if(nodeName.empty()) {
            node = writeNode;
        } else {
            node[nodeName] = writeNode;
        }
    }

    return addedNode;
}
"

        echo "$READ_REQUIRED" >>$OUT_FILE
        echo "$READ_OPTIONAL" >>$OUT_FILE
        echo "$WRITE_REQUIRED" >>$OUT_FILE
        echo "$WRITE_OPTIONAL" >>$OUT_FILE

    else
        # New struct member
        TYPE=$(awk '{print $1;}' <<<$LINE)
        VAR="$(awk 'NF>1{print $NF}' <<<$LINE)"
        VAR="${VAR//;/}"

        if [[ $LINE = *"#"* ]]; then
            :

        elif [[ "$VAR" = "pNext" ]]; then
            READ_REQUIRED="$READ_REQUIRED
        // void* - pNext
        newData.pNext = nullptr;
"

            READ_OPTIONAL="$READ_OPTIONAL
        // void* - pNext
        newData.pNext = nullptr;
"

        elif [[ "$TYPE" = "VkStructureType" ]]; then
            VALUE=$(awk '{print $3;}' <<<$LINE)
            if [[ "$VALUE" = "" ]]; then
                echo "ERROR: VkStructureType doesn't have corresponding value!"
                exit 1
            fi
            READ_REQUIRED="$READ_REQUIRED
        // $TYPE - sType
        newData.sType = $VALUE;
"

            READ_OPTIONAL="$READ_OPTIONAL
        // $TYPE - sType
        newData.sType = $VALUE;
"

        elif [[ "$TYPE" = "const" ]] && [[ "$LINE" = *"*"* ]]; then
            # It's an array of lower-level structs
            TYPE=$(awk '{print $2;}' <<<$LINE)
            TYPE=${TYPE%?}
            NAME="$(echo ${VAR:1:1} | tr '[:upper:]' '[:lower:]')${VAR:2}"
            PTR_MEMBERS="$PTR_MEMBERS $VAR"
            COUNT_NAME=${NAME%?}Count

            READ_REQUIRED="$READ_REQUIRED
        // $TYPE - $VAR / $COUNT_NAME
        std::unique_ptr<$TYPE[]> $VAR;
        if (auto ${NAME}Node = subNode[\"$NAME\"]; ${NAME}Node) {
            newData.$VAR = static_cast<$TYPE *>(calloc(${NAME}Node.size(), sizeof($TYPE)));
            size_t count = 0;
            for (auto it = ${NAME}Node.begin(); it != ${NAME}Node.end(); ++it) {
                yaml_read_required(\"\", *it, *const_cast<$TYPE *>(&newData.$VAR[count]));
                ++count;
            }
            newData.${COUNT_NAME} = static_cast<uint32_t>(${NAME}Node.size());
            read = true;
        } else {
            throw foeYamlException{\"${NAME} - Required node not found\"};
        }
"

            READ_OPTIONAL="$READ_OPTIONAL
        // $TYPE - $VAR / $COUNT_NAME
        std::unique_ptr<$TYPE[]> $VAR;
        if (auto ${NAME}Node = subNode[\"$NAME\"]; ${NAME}Node) {
            newData.$VAR = static_cast<$TYPE *>(calloc(${NAME}Node.size(), sizeof($TYPE)));
            size_t count = 0;
            for (auto it = ${NAME}Node.begin(); it != ${NAME}Node.end(); ++it) {
                yaml_read_required(\"\", *it, *const_cast<$TYPE *>(&newData.$VAR[count]));
                ++count;
            }
            newData.${COUNT_NAME} = static_cast<uint32_t>(${NAME}Node.size());
            read = true;
        }
"

            WRITE_REQUIRED="$WRITE_REQUIRED
        // $TYPE - $VAR / $COUNT_NAME
        YAML::Node ${NAME}Node;
        for(uint32_t i = 0; i < data.${COUNT_NAME}; ++i) {
            YAML::Node newNode;
            yaml_write_required<$TYPE>(\"\", data.$VAR[i], newNode);
            ${NAME}Node.push_back(newNode);
        }
        writeNode[\"$NAME\"] = ${NAME}Node;
"

            WRITE_OPTIONAL="$WRITE_OPTIONAL
        // $TYPE - $VAR / $COUNT_NAME
        if(data.${COUNT_NAME} > 0) {
            YAML::Node ${NAME}Node;
            for(uint32_t i = 0; i < data.${COUNT_NAME}; ++i) {
                YAML::Node newNode;
                yaml_write_required<$TYPE>(\"\", data.$VAR[i], newNode);
                ${NAME}Node.push_back(newNode);
            }
            writeNode[\"$NAME\"] = ${NAME}Node;
            addedNode = true;
        }
"

        elif [[ "$TYPE" = *"Vk"* ]] && [[ "$TYPE" = *"Flags"* ]]; then
            # It's a flag type which is typically an overloaded basic type
            READ_REQUIRED="$READ_REQUIRED
        // $TYPE - $VAR
        read |= yaml_read_optional_vk<$TYPE>(\"$TYPE\", \"$VAR\", subNode, newData.$VAR);
"

            READ_OPTIONAL="$READ_OPTIONAL
        // $TYPE - $VAR
        read |= yaml_read_optional_vk<$TYPE>(\"$TYPE\", \"$VAR\", subNode, newData.$VAR);
"

            WRITE_REQUIRED="$WRITE_REQUIRED
        // $TYPE - $VAR
        yaml_write_required_vk<$TYPE>(\"$TYPE\", \"$VAR\", data.$VAR, writeNode);
"

            WRITE_OPTIONAL="$WRITE_OPTIONAL
        // $TYPE - $VAR
        addedNode |= yaml_write_optional_vk<$TYPE>(\"$TYPE\", \"$VAR\", data.$VAR, defaultData.$VAR, writeNode);
"

        else
            # It's another VK-specific non-overloaded type
            READ_REQUIRED="$READ_REQUIRED
        // $TYPE - $VAR
        read |= yaml_read_optional<$TYPE>(\"$VAR\", subNode, newData.$VAR);
"

            READ_OPTIONAL="$READ_OPTIONAL
        // $TYPE - $VAR
        read |= yaml_read_optional<$TYPE>(\"$VAR\", subNode, newData.$VAR);
"

            WRITE_REQUIRED="$WRITE_REQUIRED
        // $TYPE - $VAR
        yaml_write_required<$TYPE>(\"$VAR\", data.$VAR, writeNode);
"

            WRITE_OPTIONAL="$WRITE_OPTIONAL
        // $TYPE - $VAR
        addedNode |= yaml_write_optional<$TYPE>(\"$VAR\", defaultData.$VAR, data.$VAR, writeNode);
"
        fi
    fi

done <$1

# Run clang-format
clang-format -i $OUT_FILE
