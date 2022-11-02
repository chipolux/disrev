import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import voidtweak

Rectangle {
    id: control
    implicitWidth: 200
    implicitHeight: gridLayout.height + 20
    color: index % 2 ? "#555" : "#444"

    property Instance inst
    property Matrix mat

    function formatAddr(pos) {
        return pos.toString(16).padStart(8, '0')
    }

    GridLayout {
        id: gridLayout
        columns: 3
        rowSpacing: 10
        columnSpacing: 10
        x: 10
        y: 10
        width: parent.width - 20

        RowLayout {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Label {
                text: `<b>Instance Offset:</b> ${formatAddr(
                          control.inst.offset)}`
                color: "#DDD"
                Layout.fillWidth: true
            }
            Label {
                text: `<b>Matrix Offset:</b> ${formatAddr(control.mat.offset)}`
                horizontalAlignment: Label.AlignRight
                color: "#DDD"
                Layout.fillWidth: true
            }
        }

        Label {
            text: "<b>Translation:</b>"
            elide: Label.ElideRight
            color: "#DDD"
            Layout.fillWidth: true
            Layout.columnSpan: 3
        }
        TextField {
            text: control.mat.x4
            Layout.fillWidth: true
            onEditingFinished: control.mat.x4 = text
        }
        TextField {
            text: control.mat.y4
            Layout.fillWidth: true
            onEditingFinished: control.mat.y4 = text
        }
        TextField {
            text: control.mat.z4
            Layout.fillWidth: true
            onEditingFinished: control.mat.z4 = text
        }

        Label {
            text: "<b>Min Bound:</b>"
            elide: Label.ElideRight
            color: "#DDD"
            Layout.fillWidth: true
            Layout.columnSpan: 3
        }
        TextField {
            text: control.inst.minX
            Layout.fillWidth: true
            onEditingFinished: control.inst.minX = text
        }
        TextField {
            text: control.inst.minY
            Layout.fillWidth: true
            onEditingFinished: control.inst.minY = text
        }
        TextField {
            text: control.inst.minZ
            Layout.fillWidth: true
            onEditingFinished: control.inst.minZ = text
        }

        Label {
            text: "<b>Max Bound:</b>"
            elide: Label.ElideRight
            color: "#DDD"
            Layout.fillWidth: true
            Layout.columnSpan: 3
        }
        TextField {
            text: control.inst.maxX
            Layout.fillWidth: true
            onEditingFinished: control.inst.maxX = text
        }
        TextField {
            text: control.inst.maxY
            Layout.fillWidth: true
            onEditingFinished: control.inst.maxY = text
        }
        TextField {
            text: control.inst.maxZ
            Layout.fillWidth: true
            onEditingFinished: control.inst.maxZ = text
        }
    }
}
