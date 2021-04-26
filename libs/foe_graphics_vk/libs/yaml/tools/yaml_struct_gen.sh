#!/usr/bin/env sh

if [ "$1" == "" ]; then
    echo "ERROR: Input file not specified!"
    exit 1
fi

OUT_FILE=$1_yaml.cpp

printf "" >$OUT_FILE

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
FOE_GFX_YAML_EXPORT void yaml_read_required<$STRUCT>(std::string const &nodeName, YAML::Node const &node, $STRUCT &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if(!subNode) {
        throw foeYamlException(nodeName + \" - Required node not found to parse as '$STRUCT'\");
    }

    bool read = false;
    try {"

        READ_OPTIONAL="template <>
FOE_GFX_YAML_EXPORT bool yaml_read_optional<$STRUCT>(std::string const &nodeName, YAML::Node const &node, $STRUCT &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if(!subNode) {
        return false;
    }

    bool read = false;
    try {"

        WRITE_REQUIRED="template <>
FOE_GFX_YAML_EXPORT void yaml_write_required<$STRUCT>(std::string const& nodeName, $STRUCT const &data, YAML::Node &node) {
    YAML::Node writeNode;

    try {"

        WRITE_OPTIONAL="template <>
FOE_GFX_YAML_EXPORT bool yaml_write_optional<$STRUCT>(std::string const& nodeName, $STRUCT const& defaultData, $STRUCT const &data, YAML::Node &node) {
    YAML::Node writeNode;
    bool addedNode = false;

    try {"

    elif [[ $LINE = *'}'* ]]; then
        # Complete the struct
        if [[ "$PTR_MEMBERS" != "" ]]; then
            READ_REQUIRED="$READ_REQUIRED
            
        // Releasing pointer members"
            READ_OPTIONAL="$READ_OPTIONAL

        // Releasing pointer members"
        fi

        for PTR_MEM in $PTR_MEMBERS; do
            READ_REQUIRED="$READ_REQUIRED
        data.$PTR_MEM = $PTR_MEM.release();"
        done
        READ_REQUIRED="$READ_REQUIRED
    } catch (foeYamlException const& e) {
        throw foeYamlException(nodeName + \"::\" + e.what());
    }
}
"

        for PTR_MEM in $PTR_MEMBERS; do
            READ_OPTIONAL="$READ_OPTIONAL
        data.$PTR_MEM = $PTR_MEM.release();"
        done
        READ_OPTIONAL="$READ_OPTIONAL
    } catch (foeYamlException const& e) {
        throw foeYamlException(nodeName + \"::\" + e.what());
    }

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

        if [[ "$LINE" = *"sType"* ]] || [[ $LINE = *"pNext"* ]] || [[ $LINE = *"#"* ]]; then
            :

        elif [[ "$TYPE" = "const" ]] && [[ "$LINE" = *"*"* ]]; then
            # It's an array of lower-level structs
            TYPE=$(awk '{print $2;}' <<<$LINE)
            TYPE=${TYPE%?}
            NAME="$(echo ${VAR:1:1} | tr '[:upper:]' '[:lower:]')${VAR:2}"
            PTR_MEMBERS="$PTR_MEMBERS $VAR"
            COUNT_NAME=${NAME%?}Count

            READ_REQUIRED="$READ_REQUIRED
        // $VAR / $COUNT_NAME
        std::unique_ptr<$TYPE[]> $VAR;
        if (auto ${NAME}Node = subNode[\"$NAME\"]; ${NAME}Node) {
            $VAR.reset(new $TYPE[${NAME}Node.size()]);
            memset($VAR.get(), 0, sizeof($TYPE) * ${NAME}Node.size());
            size_t count = 0;
            for (auto it = ${NAME}Node.begin(); it != ${NAME}Node.end(); ++it) {
                yaml_read_required(\"\", *it, $VAR[count]);
                ++count;
            }
            data.bindingCount = ${NAME}Node.size();
            read = true;
        } else {
            throw foeYamlException{\"${NAME} - Required node not found\"};
        }"

            READ_OPTIONAL="$READ_OPTIONAL
        // $VAR / $COUNT_NAME
        std::unique_ptr<$TYPE[]> $VAR;
        if (auto ${NAME}Node = subNode[\"$NAME\"]; ${NAME}Node) {
            $VAR.reset(new $TYPE[${NAME}Node.size()]);
            memset($VAR.get(), 0, sizeof($TYPE) * ${NAME}Node.size());
            size_t count = 0;
            for (auto it = ${NAME}Node.begin(); it != ${NAME}Node.end(); ++it) {
                yaml_read_required(\"\", *it, $VAR[count]);
                ++count;
            }
            data.bindingCount = ${NAME}Node.size();
            read = true;
        }"

            WRITE_REQUIRED="$WRITE_REQUIRED
        // $VAR / $COUNT_NAME
        YAML::Node ${NAME}Node;
        for(uint32_t i = 0; i < data.${COUNT_NAME}; ++i) {
            YAML::Node newNode;
            yaml_write_required<$TYPE>(\"\", data.$VAR[i], newNode);
            ${NAME}Node.push_back(newNode);
        }
        writeNode[\"$NAME\"] = ${NAME}Node;"

            WRITE_OPTIONAL="$WRITE_OPTIONAL
        // $VAR / $COUNT_NAME
        if(data.${COUNT_NAME} > 0) {
            YAML::Node ${NAME}Node;
            for(uint32_t i = 0; i < data.${COUNT_NAME}; ++i) {
                YAML::Node newNode;
                yaml_write_required<$TYPE>(\"\", data.$VAR[i], newNode);
                ${NAME}Node.push_back(newNode);
            }
            writeNode[\"$NAME\"] = ${NAME}Node;
            addedNode = true;
        }"

        elif [[ "$TYPE" = *"Vk"* ]] && [[ "$TYPE" = *"Flags"* ]]; then
            # It's a flag type which is typically an overloaded basic type
            READ_REQUIRED="$READ_REQUIRED
        read |= yaml_read_optional_vk<$TYPE>(\"$TYPE\", \"$VAR\", subNode, data.$VAR);"

            READ_OPTIONAL="$READ_OPTIONAL
        read |= yaml_read_optional_vk<$TYPE>(\"$TYPE\", \"$VAR\", subNode, data.$VAR);"

            WRITE_REQUIRED="$WRITE_REQUIRED
        yaml_write_required_vk<$TYPE>(\"$TYPE\", \"$VAR\", data.$VAR, writeNode);"

            WRITE_OPTIONAL="$WRITE_OPTIONAL
        addedNode |= yaml_write_optional_vk<$TYPE>(\"$TYPE\", \"$VAR\", data.$VAR, defaultData.$VAR, writeNode);"

        else
            # It's another VK-specific non-overloaded type
            READ_REQUIRED="$READ_REQUIRED
        read |= yaml_read_optional<$TYPE>(\"$VAR\", subNode, data.$VAR);"

            READ_OPTIONAL="$READ_OPTIONAL
        read |= yaml_read_optional<$TYPE>(\"$VAR\", subNode, data.$VAR);"

            WRITE_REQUIRED="$WRITE_REQUIRED
        yaml_write_required<$TYPE>(\"$VAR\", data.$VAR, writeNode);"

            WRITE_OPTIONAL="$WRITE_OPTIONAL
        addedNode |= yaml_write_optional<$TYPE>(\"$VAR\", defaultData.$VAR, data.$VAR, writeNode);"
        fi
    fi

done <$1
