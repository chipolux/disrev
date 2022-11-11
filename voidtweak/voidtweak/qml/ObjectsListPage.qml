import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import voidtweak

Item {
    id: outerPage

    property string filter
    property Object obj

    function formatAddr(pos) {
        return pos.toString(16).padStart(8, '0')
    }

    Loader {
        sourceComponent: !!obj ? editComponent : listComponent
        anchors.fill: parent
    }

    Component {
        id: listComponent

        Item {
            id: page

            ListView {
                id: objectList
                model: !outerPage.filter ? core.objects : core.objects.filter(
                                               o => o.materialPath.includes(
                                                   outerPage.filter))
                enabled: !core.busy
                interactive: !contextMenu.visible
                boundsBehavior: ListView.StopAtBounds
                anchors.top: filterInput.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 5

                delegate: ObjectListItem {
                    width: ListView.view.width
                    obj: modelData

                    MouseArea {
                        anchors.fill: parent

                        onClicked: outerPage.obj = modelData
                    }
                }
            }

            Button {
                id: clearButton
                height: filterInput.height
                text: "Back"
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.margins: 5

                onClicked: core.clearObjects()
            }

            TextField {
                id: filterInput
                placeholderText: "Filter objects..."
                selectByMouse: true
                selectionColor: "orange"
                anchors.left: clearButton.right
                anchors.right: saveButton.left
                anchors.top: parent.top
                anchors.margins: 5

                onAccepted: {
                    outerPage.filter = text
                }
            }

            Button {
                id: saveButton
                height: filterInput.height
                text: "Save"
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 5

                onClicked: core.saveObjects()
            }

            Menu {
                id: contextMenu

                MenuItem {
                    text: "Is This Necessary?"
                }
            }
        }
    }

    Component {
        id: editComponent

        Item {
            id: page

            ListView {
                id: objectList
                model: outerPage.obj.instances
                enabled: !core.busy
                boundsBehavior: ListView.StopAtBounds
                anchors.top: clearButton.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 5

                delegate: InstanceListItem {
                    width: ListView.view.width
                    inst: modelData
                    mat: outerPage.obj.matrices[index]
                }
            }

            Button {
                id: clearButton
                text: "Back"
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.margins: 5

                onClicked: outerPage.obj = null
            }

            Label {
                text: `Object ${formatAddr(
                          outerPage.obj.offset)}, ${outerPage.obj.materialPath}`
                color: "#ddd"
                horizontalAlignment: Label.AlignHCenter
                anchors.left: clearButton.right
                anchors.right: saveButton.left
                anchors.verticalCenter: clearButton.verticalCenter
            }

            Button {
                id: saveButton
                text: "Save"
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 5

                onClicked: {
                    forceActiveFocus()
                    core.saveObject(outerPage.obj)
                }
            }
        }
    }
}
