import QtQuick
import QtQuick.Controls
import voidtweak

Rectangle {
    id: control
    implicitWidth: 200
    implicitHeight: 30
    color: index % 2 ? "#555" : "#444"
    radius: 4
    border.width: selected ? 2 : 0
    border.color: "orange"

    signal editRequested
    property Entity entity
    property bool selected: false

    Label {
        id: idLabel
        text: `${entity.entityType} ${entity.entityId}`
        elide: Label.ElideRight
        color: "#DDD"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 10
    }

    MouseArea {
        anchors.fill: parent

        onClicked: control.editRequested()
    }
}
