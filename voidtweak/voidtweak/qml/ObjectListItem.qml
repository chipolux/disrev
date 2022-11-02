import QtQuick
import QtQuick.Controls
import voidtweak

Rectangle {
    id: control
    implicitWidth: 200
    implicitHeight: 80
    color: index % 2 ? "#555" : "#444"

    property Object obj

    function formatAddr(pos) {
        return pos.toString(16).padStart(8, '0')
    }

    Column {
        id: leftColumn
        spacing: 5
        anchors.left: parent.left
        anchors.right: rightColumn.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 10

        Label {
            text: `<b>Offset:</b> ${formatAddr(obj.offset)}`
            elide: Label.ElideRight
            color: "#DDD"
            width: parent.width
        }
        Label {
            text: `<b>Material:</b> ${obj.materialPath}`
            elide: Label.ElideRight
            color: "#DDD"
            width: parent.width
        }
        Label {
            text: `<b>Mesh:</b> ${obj.meshIndex}`
            elide: Label.ElideRight
            color: "#DDD"
            width: parent.width
        }
    }

    Column {
        id: rightColumn
        spacing: 5
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 10

        Label {
            text: `<b>LOD:</b> ${obj.lod}`
            horizontalAlignment: Label.AlignRight
            color: "#DDD"
        }
        Label {
            text: `<b>Matrices:</b> ${obj.matrices.length}`
            horizontalAlignment: Label.AlignRight
            color: "#DDD"
        }
        Label {
            text: `<b>Instances:</b> ${obj.instances.length}`
            horizontalAlignment: Label.AlignRight
            color: "#DDD"
        }
    }
}
