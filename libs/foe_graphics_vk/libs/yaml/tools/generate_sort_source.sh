#!/usr/bin/env sh

YAML_FILE="$1"

for STRUCT in $(cat $YAML_FILE | grep -e '^Vk.*:$'); do
    STRUCT="${STRUCT::-1}"
    printf "    SORT_MACRO($STRUCT)\n"
done
