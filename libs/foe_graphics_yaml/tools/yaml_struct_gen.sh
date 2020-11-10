#!/usr/bin/env sh

OUT_FILE=$1_yaml.cpp

printf "" >$OUT_FILE

READ_REQUIRED=
READ_OPTIONAL=
WRITE_REQUIRED=
WRITE_OPTIONAL=

while read LINE; do
    if [[ $LINE = *"struct"* ]]; then
        # Start of a new struct
        STRUCT="$(cut -d ' ' -f3 <<<"$LINE")"

        READ_REQUIRED="template <>
FOE_GFX_YAML_EXPORT bool yaml_read_required<$STRUCT>(std::string const &nodeName, YAML::Node const &node, $STRUCT &data) {
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
FOE_GFX_YAML_EXPORT bool yaml_write_required<$STRUCT>(std::string const& nodeName, $STRUCT const &data, YAML::Node &node) {
    YAML::Node writeNode;

    try {"

        WRITE_OPTIONAL="template <>
FOE_GFX_YAML_EXPORT bool yaml_write_optional<$STRUCT>(std::string const& nodeName, $STRUCT const& defaultData, $STRUCT const &data, YAML::Node &node) {
    YAML::Node writeNode;
    bool addedNode = false;

    try {"

    elif [[ $LINE = *'}'* ]]; then
        # Complete the struct
        READ_REQUIRED="$READ_REQUIRED
    } catch (foeYamlException const& e) {
        throw foeYamlException(nodeName + \"::\" + e.what());
    }

    return read;
}
"

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

    return true;
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

        if [[ "$LINE" = *"sType"* ]] || [[ $LINE = *"pNext"* ]] || [[ $LINE = *"*"* ]]; then
            :

        elif [[ "$TYPE" = *"Vk"* ]] && [[ "$TYPE" = *"Flags"* ]]; then
            READ_REQUIRED="$READ_REQUIRED
        read |= yaml_read_optional_vk<$TYPE>(\"$TYPE\", \"$VAR\", subNode, data.$VAR);"

            READ_OPTIONAL="$READ_OPTIONAL
        read |= yaml_read_optional_vk<$TYPE>(\"$TYPE\", \"$VAR\", subNode, data.$VAR);"

            WRITE_REQUIRED="$WRITE_REQUIRED
        yaml_write_required_vk<$TYPE>(\"$TYPE\", \"$VAR\", data.$VAR, writeNode);"

            WRITE_OPTIONAL="$WRITE_OPTIONAL
        addedNode |= yaml_write_optional_vk<$TYPE>(\"$TYPE\", \"$VAR\", data.$VAR, defaultData.$VAR, writeNode);"

        else
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
