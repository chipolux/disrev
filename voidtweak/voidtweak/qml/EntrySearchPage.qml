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

    RowLayout {
        id: inputRow
        spacing: 5
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 5

        TextField {
            placeholderText: "Search in source and destination..."
            selectByMouse: true
            selectionColor: "orange"
            enabled: !core.busy

            onAccepted: core.search(text)

            Layout.fillWidth: true
        }

        Button {
            text: "Load Indexes"
            enabled: !core.busy

            onClicked: core.loadIndexes()

            Layout.fillHeight: true
        }

        Button {
            text: "Export All"
            enabled: !core.busy && !!core.entryCount

            onClicked: folderDialog.open()

            Layout.fillHeight: true
        }

        Button {
            text: "Launch Game"
            enabled: !core.busy && !!core.entryCount

            onClicked: core.launchGame()

            Layout.fillHeight: true
        }
    }

    Label {
        id: searchStatusLabel
        text: `Found ${core.resultCount} results...`
        color: "#DDD"
        anchors.left: parent.left
        anchors.top: inputRow.bottom
        anchors.margins: 5
    }

    Label {
        id: sortLabel
        text: `Sort: ${core.sortOrderName}`
        color: "#DDD"
        anchors.right: parent.right
        anchors.top: inputRow.bottom
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
        MenuItem {
            text: "Load BWM"
            visible: contextMenu.entry && contextMenu.entry.dstSuffix === "bwm"
            height: visible ? undefined : 0
            onTriggered: core.loadBwm(contextMenu.entry)
        }
    }

    FileDialog {
        id: fileDialog
        currentFolder: settings.lastFolder

        property Entry entry

        onAccepted: {
            if (fileDialog.fileMode === FileDialog.SaveFile
                    && !!fileDialog.entry) {
                core.exportEntry(fileDialog.entry, fileDialog.selectedFile)
                // NOTE: remove the filename, if it does not exist filedialog is upset
                var url = String(fileDialog.selectedFile)
                fileDialog.currentFolder = url.substring(0,
                                                         url.lastIndexOf("/"))
            }
            if (fileDialog.fileMode === FileDialog.OpenFile
                    && !!fileDialog.entry) {
                core.importEntry(fileDialog.entry, fileDialog.selectedFile)
                fileDialog.currentFolder = fileDialog.selectedFile
            }
        }
        onCurrentFolderChanged: settings.lastFolder = currentFolder
    }

    FolderDialog {
        id: folderDialog
        currentFolder: settings.lastFolder

        onAccepted: core.exportAllEntries(folderDialog.selectedFolder)
        onCurrentFolderChanged: settings.lastFolder = currentFolder
    }

    Settings {
        id: settings

        property url lastFolder: StandardPaths.writableLocation(
                                     StandardPaths.HomeLocation)
    }
}
