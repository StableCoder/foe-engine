#!/usr/bin/env bash

# Variables
DEVICE_NAME=
DEVICE_INDEX=0

# Command-line options
while [[ $# -gt 0 ]]; do
    case $1 in
    --device-name)
        DEVICE_NAME=$2
        shift
        shift
        ;;
    --device-index)
        DEVICE_INDEX=$2
        shift
        shift
        ;;
    *)
        echo "ERROR: Unknown option ($1)"
        exit 1
        ;;
    esac
done

# Input Checking
if [ -z $DEVICE_NAME ]; then
    echo "ERROR: No device name specificed"
    exit 1
fi

# Device work
DEVICE_COUNT=$(vulkaninfo | grep "deviceName" | cut -d '=' -f2 | wc -l)
if [[ $DEVICE_INDEX -ge $DEVICE_COUNT ]]; then
    echo "ERROR: Selected device out of range (Index: $DEVICE_INDEX)"
    exit 1
fi

VK_DEVICE_NAME=$(vulkaninfo | grep "deviceName" | cut -d '=' -f2 | head -n$((DEVICE_INDEX + 1)) | tail -n1 | xargs)

# Matching
if [[ "$DEVICE_NAME" == "llvmpipe" ]]; then
    if [[ "$VK_DEVICE_NAME" == *"llvmpipe"* ]]; then
        exit 0
    fi
elif [[ "$DEVICE_NAME" == "vega7" ]]; then
    if [[ "$VK_DEVICE_NAME" == *"RADV RENOIR"* ]]; then
        exit 0
    fi
elif [[ "$DEVICE_NAME" == "navi21" ]]; then
    if [[ "$VK_DEVICE_NAME" == *"RADV NAVI21"* ]] || [[ "$VK_DEVICE_NAME" == *"RADV SIENNA_CICHLID"* ]]; then
        exit 0
    fi
elif [[ "$DEVICE_NAME" == "navi23" ]]; then
    if [[ "$VK_DEVICE_NAME" == *"RADV NAVI23"* ]] || [[ "$VK_DEVICE_NAME" == *"RADV DIMGREY_CAVEFISH"* ]]; then
        exit 0
    fi
elif [[ "$DEVICE_NAME" == "apple_m1" ]]; then
    if [[ "$VK_DEVICE_NAME" == *"Apple M1"* ]]; then
        exit 0
    fi
fi

# Exit with error if didn't match above
echo "ERROR: No match for device name '$DEVICE_NAME' with selected index of $DEVICE_INDEX"
vulkaninfo
exit 1
