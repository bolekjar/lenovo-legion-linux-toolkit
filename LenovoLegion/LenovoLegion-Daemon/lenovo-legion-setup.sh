#!/bin/bash
# Setup of group legion, members of this group are allowed to connect to LenovoLegion-Daemon
#
# Usage: sudo lenovo-legion-setup.sh [user] [-y]

GROUP=legion
SYSUSERS_CONF=/usr/lib/sysusers.d/lenovo-legion.conf
ASSUME_YES=0
TARGET_USER=""

for arg in "$@"; do
    case "$arg" in
        -y|--yes) ASSUME_YES=1 ;;
        -h|--help)
            echo "Usage: sudo $0 [user] [-y]"
            echo "Adds user (default: user who ran sudo) to group '$GROUP' to allow using LenovoLegion GUI."
            exit 0
            ;;
        *) TARGET_USER="$arg" ;;
    esac
done

if [ "$(id -u)" -ne 0 ]; then
    echo "This script must be run as root: sudo $0 $*" >&2
    exit 1
fi

ask() {
    # ask "question" -> 0 for yes (default), 1 for no
    if [ "$ASSUME_YES" -eq 1 ]; then
        return 0
    fi
    if [ ! -t 0 ]; then
        echo "$1 [Y/n] -> no terminal, use -y to confirm" >&2
        return 1
    fi
    read -r -p "$1 [Y/n] " answer
    case "$answer" in
        ""|[Yy]|[Yy][Ee][Ss]) return 0 ;;
        *) return 1 ;;
    esac
}

# Create group if missing
if ! getent group "$GROUP" > /dev/null; then
    if [ -f "$SYSUSERS_CONF" ] && command -v systemd-sysusers > /dev/null; then
        systemd-sysusers "$SYSUSERS_CONF" || exit 1
    else
        groupadd --system "$GROUP" || exit 1
    fi
    echo "Group '$GROUP' was created."
fi

# Select user
if [ -z "$TARGET_USER" ]; then
    TARGET_USER="${SUDO_USER:-}"
fi

if [ -z "$TARGET_USER" ] || [ "$TARGET_USER" = "root" ]; then
    if [ ! -t 0 ]; then
        echo "No user given, run: sudo $0 <user>" >&2
        exit 1
    fi
    read -r -p "Which user should be allowed to use LenovoLegion GUI? " TARGET_USER
fi

if ! id "$TARGET_USER" > /dev/null 2>&1; then
    echo "User '$TARGET_USER' does not exist." >&2
    exit 1
fi

# Add user to group
if id -nG "$TARGET_USER" | tr ' ' '\n' | grep -qx "$GROUP"; then
    echo "User '$TARGET_USER' is already member of group '$GROUP'."
    exit 0
fi

if ask "Add user '$TARGET_USER' to group '$GROUP' to allow using LenovoLegion GUI?"; then
    usermod -aG "$GROUP" "$TARGET_USER" || exit 1
    echo "User '$TARGET_USER' was added to group '$GROUP'."
    echo "Please log out and log in again (or reboot) to apply the change."
else
    echo "Skipped. You can run this script again later: sudo $0"
fi
