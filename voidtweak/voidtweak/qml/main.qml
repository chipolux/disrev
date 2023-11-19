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
        source: {
            if (!!core.entities.length) {
                return "EntitiesListPage.qml"  // .entities files edit page
            } else if (!!core.objects.length) {
                return "ObjectsListPage.qml"  // 3d "objects" edit page
            } else {
                return "EntrySearchPage.qml" // main page resource file "entries"
            }
        }
        anchors.fill: parent
    }

    Rectangle {
        color: "#000"
        opacity: core.busy ? 0.3 : 0.0
        anchors.fill: parent

        Behavior on opacity {
            OpacityAnimator {
                duration: 250
            }
        }
    }

    AnimatedImage {
        width: Math.min(parent.width * 0.5, parent.height * 0.5)
        height: width
        opacity: core.busy ? 1.0 : 0.0
        smooth: false
        speed: 1.5
        source: "/voidtweak/resources/rune-spinner.gif"
        anchors.centerIn: parent

        Behavior on opacity {
            OpacityAnimator {
                duration: 250
            }
        }
    }
}
