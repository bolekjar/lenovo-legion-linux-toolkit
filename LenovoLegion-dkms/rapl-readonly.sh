#!/bin/bash
# Helper script for udev RAPL readonly rules

if [ -z "$1" ]; then
    exit 1
fi

# Make the file readonly if it exists
if [ -e "$1" ]; then
    /bin/chmod -f 0444 "$1"
fi
