import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: rootWindow
    minimumWidth: 800
    minimumHeight: 400
    width: 1280
    height: 720
    visible: true
    title: `${Qt.application.displayName} (v${Qt.application.version})`
    color: "#333"

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

    ListView {
        id: resultsList
        model: core.results
        spacing: 2
        anchors.top: controlColumn.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: errorLabel.top
        anchors.margins: 5

        delegate: Rectangle {
            width: ListView.view.width
            height: 80
            color: index % 2 ? "#555" : "#444"
            radius: 4

            Column {
                spacing: 5
                anchors.fill: parent
                anchors.margins: 10

                Label {
                    text: `<b>Source:</b> ${modelData.src}`
                    color: "#DDD"
                }
                Label {
                    text: `<b>Destination:</b> ${modelData.dst}`
                    color: "#DDD"
                }
                Label {
                    text: `<b>Size:</b> ${formatBytes(modelData.size)}`
                    color: "#DDD"
                }
            }
        }
    }

    Rectangle {
        // gradient under control column
        gradient: Gradient {
            GradientStop {
                position: 0.8
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
        anchors.topMargin: -20
    }

    Column {
        id: controlColumn
        spacing: 5
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 5

        Button {
            text: core.loading ? "Loading Indexes..." : "Load Indexes"
            enabled: !core.loading
            anchors.horizontalCenter: parent.horizontalCenter

            onClicked: core.loadIndexes()
        }

        Label {
            text: `Loaded ${core.entryCount} entries from ${core.containerCount} containers.`
            color: "#DDD"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        TextField {
            placeholderText: "Search in source and destination..."
            width: parent.width
            enabled: !core.loading

            onAccepted: core.search(text)
        }

        Label {
            text: `Found ${core.resultCount} results...`
            color: "#DDD"
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    Label {
        id: errorLabel
        text: core.error
        visible: !!core.error
        color: "orange"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.margins: 5
    }
}
