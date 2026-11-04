// SPDX-License-Identifier: GPL-2.0
// Section with all fields of a data type in the order of the .proto (fields not sent by the daemon are hidden)
import QtQuick
import QtQuick.Layouts
import LenovoLegion.Ui 1.0

Section {
    id: autoSection

    property string dataType
    property bool   autoApply: false

    source: DataSource {
        dataType: autoSection.dataType
        autoApply: autoSection.autoApply
    }

    Repeater {
        model: autoSection.source.fieldPaths

        delegate: Field {
            Layout.fillWidth: true

            source: autoSection.source
            path: modelData
        }
    }
}
