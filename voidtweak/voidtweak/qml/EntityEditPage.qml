import QtQuick
import QtQuick.Controls
import voidtweak

Item {
    id: page

    property Entity entity

    Rectangle {
        id: entriesContainer
        color: "#444"
        radius: 4
        width: parent.width
        clip: true
        anchors.top: infoContainer.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.topMargin: 3

        Flickable {
            id: entriesFlickable
            boundsBehavior: Flickable.StopAtBounds
            contentHeight: entriesColumn.height
            contentWidth: width
            anchors.fill: parent
            anchors.margins: 5

            Column {
                id: entriesColumn
                width: parent.width

                Repeater {
                    model: !!page.entity ? page.entity.entries : undefined

                    EntityEntryListItem {
                        width: parent.width
                        entry: modelData
                    }
                }
            }
        }
    }

    Rectangle {
        id: infoContainer
        height: 30
        width: parent.width
        color: "#444"
        radius: 4

        Label {
            text: !!entity ? `${entity.entityType} ${entity.entityId}` : "Select one entity to edit..."
            color: "#DDD"
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 10
        }
    }
}
