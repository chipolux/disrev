import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: rootWindow
    minimumWidth: 800
    minimumHeight: 400
    width: 1280
    height: 720
    visible: true
    title: `${Qt.application.displayName} (v${Qt.application.version})`
    color: "#333"

    Loader {
        source: "EntrySearchPage.qml"
        anchors.fill: parent
    }
}
