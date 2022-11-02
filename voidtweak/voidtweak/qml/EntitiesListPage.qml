import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import voidtweak

Item {
    id: page

    Keys.onUpPressed: entitiesList.flick(0, 800)
    Keys.onDownPressed: entitiesList.flick(0, -800)
    Keys.onEscapePressed: page.indexes = []

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
        entity: page.indexes.length === 1 && entitiesList.model.length
                > page.indexes[0] ? entitiesList.model[page.indexes[0]] : null
        anchors.top: entitiesList.top
        anchors.bottom: entitiesList.bottom
        anchors.right: parent.right
        anchors.rightMargin: 5

        onEditEntry: function (entry) {
            popup.entry = entry
            popup.open()
        }
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
        selectByMouse: true
        selectionColor: "orange"
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

            onTriggered: {
                core.deleteEntities(page.indexes.map(
                                        i => entitiesList.model[i]))
                page.indexes = []
            }
        }
    }

    Popup {
        id: popup
        modal: true
        anchors.centerIn: parent

        property EntityEntry entry
        property int entriesCount: !!popup.entry ? popup.entry.entries.length : 0

        onClosed: {
            keyInput.clear()
            valueInput.clear()
            popup.entry = null
        }

        ColumnLayout {
            spacing: 5

            Label {
                text: "Key:"
            }

            TextField {
                id: keyInput
                text: !!popup.entry ? popup.entry.key : ""
                selectByMouse: true
                selectionColor: "orange"
                Layout.minimumWidth: 500
            }

            Label {
                text: !popup.entriesCount ? "Value:" : `Entries: ${popup.entriesCount}`
            }

            TextField {
                id: valueInput
                text: !!popup.entry ? popup.entry.value : ""
                visible: !popup.entriesCount
                selectByMouse: true
                selectionColor: "orange"
                Layout.fillWidth: true
            }

            Button {
                text: "Save"
                Layout.fillWidth: true

                onClicked: {
                    forceActiveFocus()
                    popup.entry.key = keyInput.text
                    popup.entry.value = valueInput.text
                    popup.close()
                }
            }

            Button {
                text: "Delete"
                Layout.fillWidth: true

                onClicked: {
                    forceActiveFocus()
                    if (popup.entry.scope) {
                        popup.entry.scope.deleteEntry(popup.entry)
                    }
                    popup.close()
                }
            }
        }
    }
}
