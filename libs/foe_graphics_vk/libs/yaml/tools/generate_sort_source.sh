# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

#!/usr/bin/env sh

YAML_FILE="$1"

for STRUCT in $(cat $YAML_FILE | grep -e '^Vk.*:$'); do
    STRUCT="${STRUCT::-1}"
    printf "    SORT_MACRO($STRUCT)\n"
done
