#!/usr/bin/env sh

YAML_FILE="$1"

for STRUCT in $(cat $YAML_FILE | grep -e '^Vk.*:$'); do
    STRUCT="${STRUCT::-1}"
    printf "    if (std::string_view{argv[1]} == \"$STRUCT\") {
        while (__AFL_LOOP(100000)) {
            FUZZ_MACRO($STRUCT)
        }
    }
"
done
