import QtQuick
import QtQuick.Controls
import voidtweak

Rectangle {
    id: control
    implicitWidth: 200
    implicitHeight: 80
    color: index % 2 ? "#555" : "#444"
    radius: 4

    signal menuRequested

    property Entry entry

    function formatBytes(bytes) {
        var size = bytes
        var unit = "B"
        if (bytes > 1024 * 1024 * 1024) {
            size = Number(bytes / 1024 / 1024 / 1024).toFixed(1)
            unit = "gb"
        } else if (bytes > 1024 * 1024) {
            size = Number(bytes / 1024 / 1024).toFixed(1)
            unit = "mb"
        } else if (bytes > 1024) {
            size = Math.floor(bytes / 1024)
            unit = "kb"
        }
        return `${size} ${unit}`
    }

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
            text: `<b>Type:</b> ${entry.type}`
            elide: Label.ElideRight
            color: "#DDD"
            width: parent.width
        }
        Label {
            text: `<b>Src:</b> ${entry.src}`
            elide: Label.ElideRight
            color: "#DDD"
            width: parent.width
        }
        Label {
            text: `<b>Dst:</b> ${entry.dst}`
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
            text: `<b>ID:</b> ${entry.id}`
            horizontalAlignment: Label.AlignRight
            color: "#DDD"
        }
        Label {
            text: `<b>Size:</b> ${formatBytes(entry.size)}`
            horizontalAlignment: Label.AlignRight
            color: "#DDD"
        }
        Label {
            text: `game${entry.container + 1} @ ${formatAddr(entry.indexPos)}`
            horizontalAlignment: Label.AlignRight
            color: "#DDD"
        }
    }

    MouseArea {
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        anchors.fill: parent

        onClicked: function (event) {
            if (event.button === Qt.RightButton) {
                control.menuRequested()
            }
        }
    }
}
