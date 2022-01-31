#!/usr/bin/env sh

YAML_FILE="$1"

for STRUCT in $(cat $YAML_FILE | grep -e '^Vk.*:$'); do
    STRUCT="${STRUCT::-1}"
    printf "
TEST_CASE(\"$STRUCT fuzzed input\") {
    FUZZ_TEST_MACRO(FUZZED_TEST_DATA_DIR, $STRUCT);
}
"
done
