import QtQuick
import QtQuick.Controls
import voidtweak

Item {
    id: page

    Keys.onUpPressed: entitiesList.flick(0, 800)
    Keys.onDownPressed: entitiesList.flick(0, -800)

    property var indexes: []
    property string filter

    ListView {
        id: entitiesList
        model: !page.filter ? core.entities : core.entities.filter(
                                  e => e.entityId.includes(page.filter))
        enabled: !core.busy
        interactive: !entityMenu.visible
        boundsBehavior: ListView.StopAtBounds
        anchors.top: filterInput.bottom
        anchors.left: parent.left
        anchors.right: entityEditPage.left
        anchors.bottom: parent.bottom
        anchors.margins: 5

        delegate: EntityListItem {
            width: ListView.view.width
            entity: modelData
            selected: page.indexes.includes(index)

            MouseArea {
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                anchors.fill: parent

                onClicked: function (event) {
                    var leftButton = event.button === Qt.LeftButton
                    var rightButton = event.button === Qt.RightButton
                    var shiftMod = event.modifiers & Qt.ShiftModifier
                    var ctrlMod = event.modifiers & Qt.ControlModifier
                    if (index === -1) {
                        return
                    } else if (rightButton && page.indexes.includes(index)) {
                        entityMenu.popup()
                    } else if (rightButton) {
                        page.indexes = [index]
                        entityMenu.popup()
                    } else if (leftButton && !shiftMod && !ctrlMod) {
                        page.indexes = [index]
                    } else if (leftButton && shiftMod) {
                        var indexes = []
                        var startIndex = !!page.indexes.length ? page.indexes[0] : index
                        var direction = index > startIndex ? 1 : -1
                        for (var i = startIndex; i !== index; i += direction) {
                            indexes.push(i)
                        }
                        indexes.push(index)
                        page.indexes = indexes
                    } else if (leftButton && ctrlMod
                               && !page.indexes.includes(index)) {
                        page.indexes.push(index)
                        page.indexesChanged()
                    }
                }
            }
        }
    }

    EntityEditPage {
        id: entityEditPage
        width: parent.width * 0.7
        entity: page.indexes.length === 1 ? core.entities[page.indexes[0]] : null
        anchors.top: entitiesList.top
        anchors.bottom: entitiesList.bottom
        anchors.right: parent.right
        anchors.rightMargin: 5
    }

    Button {
        id: clearButton
        height: filterInput.height
        text: "Back"
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 5

        onClicked: core.clearEntities()
    }

    TextField {
        id: filterInput
        placeholderText: "Filter entities..."
        anchors.left: clearButton.right
        anchors.right: saveButton.left
        anchors.top: parent.top
        anchors.margins: 5

        onAccepted: {
            page.indexes = []
            page.filter = text
        }
    }

    Button {
        id: saveButton
        height: filterInput.height
        text: "Save"
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 5

        onClicked: core.saveEntities()
    }

    Menu {
        id: entityMenu

        MenuItem {
            text: "Copy"
            height: visible ? undefined : 0
            visible: page.indexes.length === 1
        }
        MenuItem {
            text: "Add Before"
            height: visible ? undefined : 0
            visible: page.indexes.length === 1
        }
        MenuItem {
            text: "Add After"
            height: visible ? undefined : 0
            visible: page.indexes.length === 1
        }
        MenuItem {
            text: page.indexes.length > 1 ? "Delete Selected" : "Delete"
            height: visible ? undefined : 0
            visible: page.indexes.length > 0

            onTriggered: core.deleteEntities(page.indexes.map(
                                                 i => entitiesList.model[i]))
        }
    }
}
