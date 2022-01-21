#!/usr/bin/env sh

# ./gen_merge_feature_set <struct> <vulkan_core.h file>
#
# This takes in the name of a Vulkan features struct and the vulkan_core file
# and prints out a function which will merge any non-false feature flags from
# the source struct and sets it to true in the destination struct.

# Variables
IN_FILE=$2
STRUCT=$1

# Figure out the struct lines
START_LINE=$(grep -n "typedef struct $STRUCT {" $IN_FILE | cut -d ':' -f 1)
END_LINE=$(grep -n "} $STRUCT;" $IN_FILE | cut -d ':' -f 1)
START_LINE=$((START_LINE + 1))
END_LINE=$((END_LINE - 1))

# Print out the function
echo "/** @brief Merges source feature flags to the destination
 * @param pSrc is a pointer to the struct of the source set of features to set in the destination
 * @param pDst is a pointer to the destintion where flags will be set
 */"
echo "void mergeFeatureSet_$STRUCT("
echo "  $STRUCT const *pSrc,"
echo "  $STRUCT *pDst) {"
echo "  // Generated from gen_merge_feature_set.sh"
while read -r LINE; do
    TYPE=$(cut -d ' ' -f 1 <<<$LINE)
    NAME=$(echo $LINE | cut -d ' ' -f2 | cut -d ';' -f 1)
    if [[ "$TYPE" = "VkBool32" ]]; then
        echo "  if(pSrc->$NAME != VK_FALSE) pDst->$NAME = VK_TRUE;"
    fi
done < <(sed -n "${START_LINE},${END_LINE}p" $IN_FILE)
echo "}"
