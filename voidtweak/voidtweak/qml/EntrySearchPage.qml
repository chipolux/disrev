import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import voidtweak

Item {
    Keys.onUpPressed: resultsList.flick(0, 800)
    Keys.onDownPressed: resultsList.flick(0, -800)

    ListView {
        id: resultsList
        model: core.results
        enabled: !core.busy
        boundsBehavior: ListView.StopAtBounds
        interactive: !contextMenu.visible
        anchors.top: searchStatusLabel.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: errorLabel.top
        anchors.margins: 5

        delegate: EntryListItem {
            width: ListView.view.width
            entry: modelData

            onMenuRequested: {
                contextMenu.entry = modelData
                contextMenu.popup()
            }
        }
    }

    Rectangle {
        // gradient under control column
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
        anchors.bottom: resultsList.top
    }

    Rectangle {
        // gradient under error label
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: "transparent"
            }
            GradientStop {
                position: 0.6
                color: rootWindow.color
            }
        }
        anchors.top: errorLabel.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.topMargin: -10
    }

    TextField {
        id: searchInput
        placeholderText: "Search in source and destination..."
        selectByMouse: true
        selectionColor: "orange"
        enabled: !core.busy
        anchors.left: parent.left
        anchors.right: loadIndexesButton.left
        anchors.top: parent.top
        anchors.margins: 5

        onAccepted: core.search(text)
    }

    Button {
        id: loadIndexesButton
        height: searchInput.height
        text: "Load Indexes"
        enabled: !core.busy
        anchors.right: exportAllButton.left
        anchors.top: parent.top
        anchors.margins: 5

        onClicked: core.loadIndexes()
    }

    Button {
        id: exportAllButton
        height: searchInput.height
        text: "Export All"
        enabled: !core.busy && !!core.entryCount
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 5

        onClicked: folderDialog.open()
    }

    Label {
        id: searchStatusLabel
        text: `Found ${core.resultCount} results...`
        color: "#DDD"
        anchors.left: parent.left
        anchors.top: searchInput.bottom
        anchors.margins: 5
    }

    Label {
        id: sortLabel
        text: `Sort: ${core.sortOrderName}`
        color: "#DDD"
        anchors.right: parent.right
        anchors.top: searchInput.bottom
        anchors.margins: 5

        MouseArea {
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            anchors.fill: parent

            onClicked: function (event) {
                if (event.button === Qt.RightButton) {
                    core.sortResults(Core.SortNone)
                } else {
                    core.sortResults(core.sortOrder + 1)
                }
            }
            onPressAndHold: core.sortResults(Core.SortNone)
        }
    }

    Label {
        id: indexStatusLabel
        text: `Indexes: ${core.containerCount}, Entries: ${core.entryCount}`
        color: "#DDD"
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 5
    }

    Label {
        id: errorLabel
        text: core.error
        visible: !!core.error
        color: "orange"
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 5
    }

    Menu {
        id: contextMenu

        property Entry entry

        MenuItem {
            text: "Export"
            onTriggered: {
                fileDialog.entry = contextMenu.entry
                fileDialog.fileMode = FileDialog.SaveFile
                fileDialog.selectedFile = contextMenu.entry.dstFileName
                fileDialog.open()
            }
        }
        MenuItem {
            text: "Import"
            onTriggered: {
                fileDialog.entry = contextMenu.entry
                fileDialog.fileMode = FileDialog.OpenFile
                fileDialog.open()
            }
        }
        MenuItem {
            text: "Load Entities"
            visible: contextMenu.entry
                     && contextMenu.entry.dstSuffix === "entities"
            height: visible ? undefined : 0
            onTriggered: core.loadEntities(contextMenu.entry)
        }
    }

    FileDialog {
        id: fileDialog
        currentFolder: StandardPaths.writableLocation(
                           StandardPaths.HomeLocation)

        property Entry entry

        onAccepted: {
            if (fileDialog.fileMode === FileDialog.SaveFile
                    && !!fileDialog.entry) {
                core.exportEntry(fileDialog.entry, fileDialog.selectedFile)
            }
            if (fileDialog.fileMode === FileDialog.OpenFile
                    && !!fileDialog.entry) {
                core.importEntry(fileDialog.entry, fileDialog.selectedFile)
            }
        }
    }

    FolderDialog {
        id: folderDialog
        currentFolder: StandardPaths.writableLocation(
                           StandardPaths.HomeLocation)
        onAccepted: core.exportAllEntries(folderDialog.selectedFolder)
    }

    BusyIndicator {
        running: core.busy
        anchors.fill: resultsList
        anchors.margins: 40
    }
}
