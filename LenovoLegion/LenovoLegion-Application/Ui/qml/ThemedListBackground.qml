// SPDX-License-Identifier: GPL-2.0
// Background of a list of the theme (QListWidget of the stylesheet), place it under a ListView
import QtQuick

Rectangle {
    radius: theme.radius
    color: theme.listBackground
    border.width: 2
    border.color: theme.groupBorder
}
