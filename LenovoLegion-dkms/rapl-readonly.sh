#!/bin/bash
# Helper script for udev RAPL readonly rules
#
# File is readable only by root and group legion, others have no access.
# energy_uj must not be readable by others (PLATYPUS side channel, CVE-2020-8694).
# sysfs refuses to open a file without any write bit for writing even for root, so other tools (also running
# as root, e.g. thermald) can not change the power limits. Only LenovoLegion-Daemon writes the limits of the
# thermal mode (CPUPower/RaplPowerLimits): it adds the owner write bit for the write and restores 0440.

GROUP=legion

if [ -z "$1" ]; then
    exit 1
fi

if [ ! -e "$1" ]; then
    exit 0
fi

if getent group "$GROUP" > /dev/null; then
    /bin/chown -f root:"$GROUP" "$1"
    /bin/chmod -f 0440 "$1"
else
    # Group is missing, readable only by root
    /bin/chown -f root:root "$1"
    /bin/chmod -f 0400 "$1"
fi
