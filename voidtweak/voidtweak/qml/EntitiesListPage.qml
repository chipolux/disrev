import QtQuick
import QtQuick.Controls
import voidtweak

Item {
    id: page

    Keys.onUpPressed: entitiesList.flick(0, 800)
    Keys.onDownPressed: entitiesList.flick(0, -800)

    property Entity entity

    ListView {
        id: entitiesList
        model: !filterInput.text ? core.entities : core.entities.filter(
                                       e => e.entityId.includes(
                                           filterInput.text))
        enabled: !core.busy
        spacing: 5
        boundsBehavior: ListView.StopAtBounds
        anchors.top: filterInput.bottom
        anchors.left: parent.left
        anchors.right: entityEditPage.left
        anchors.bottom: parent.bottom
        anchors.margins: 5

        delegate: EntityListItem {
            width: ListView.view.width
            entity: modelData
            selected: page.entity === modelData

            onEditRequested: page.entity = modelData
        }
    }

    EntityEditPage {
        id: entityEditPage
        width: parent.width * 0.7
        entity: page.entity
        anchors.top: entitiesList.top
        anchors.bottom: entitiesList.bottom
        anchors.right: parent.right
        anchors.rightMargin: 5
    }

    Rectangle {
        gradient: Gradient {
            GradientStop {
                position: 0.85
                color: rootWindow.color
            }
            GradientStop {
                position: 1.0
                color: "transparent"
            }
        }
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: entitiesList.top
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
}
