import QtQuick
import QtQuick.Controls
import voidtweak

Item {
    id: control

    property alias zoomLevel: zoomSlider.value
    property int fontSize: {
        if (control.zoomLevel < 0.25) {
            return 6
        } else if (control.zoomLevel < 0.5) {
            return 12
        } else if (control.zoomLevel < 0.75) {
            return 16
        } else {
            return 18
        }
    }
    property real nodePadding: 80
    property real xOffset: Math.abs(
                               core.script.boundingBox.x * control.zoomLevel) + control.nodePadding
    property real yOffset: Math.abs(
                               core.script.boundingBox.y * control.zoomLevel) + control.nodePadding

    ScrollView {
        width: parent.width
        height: parent.height
        contentWidth: (core.script.boundingBox.width * control.zoomLevel)
                      + (control.nodePadding * 2)
        contentHeight: (core.script.boundingBox.height * control.zoomLevel)
                       + (control.nodePadding * 2)
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOn
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn

        Repeater {
            model: core.script.comments

            Rectangle {
                x: control.xOffset + modelData.rect.x * control.zoomLevel
                y: control.yOffset + modelData.rect.y * control.zoomLevel
                width: modelData.rect.width * control.zoomLevel
                height: modelData.rect.height * control.zoomLevel
                color: "transparent"
                border.color: "darkorange"
                border.width: 2

                Rectangle {
                    width: parent.width
                    height: 20 * control.zoomLevel
                    color: "darkorange"

                    MouseArea {
                        width: parent.width
                        height: parent.height

                        onClicked: {
                            console.info(`Comment ${modelData.id}`)
                            console.info(`   rect: ${modelData.rect}`)
                            console.info(`   outsideText: ${modelData.outsideText}`)
                            console.info(`   insideText: ${modelData.insideText}`)
                        }
                    }
                }

                Label {
                    text: modelData.outsideText
                    font.pixelSize: control.fontSize
                    font.bold: true
                    color: "darkorange"
                    anchors.bottom: parent.top
                }
            }
        }

        Repeater {
            model: core.script.nodes

            Rectangle {
                x: control.xOffset + modelData.pos.x * control.zoomLevel
                y: control.yOffset + modelData.pos.y * control.zoomLevel
                height: control.fontSize
                width: height
                color: "orchid"

                MouseArea {
                    width: parent.width
                    height: parent.height

                    onClicked: {
                        console.info(`Node ${modelData.id}`)
                        console.info(`   pos: ${modelData.pos}`)
                        console.info(`   name: ${modelData.name}`)
                    }
                }

                Label {
                    x: parent.width
                    text: modelData.name
                    font.pixelSize: control.fontSize - 2
                    font.bold: true
                    color: "darkorchid"
                }
            }
        }
    }

    Slider {
        id: zoomSlider
        from: 0.1
        to: 1.0
        stepSize: 0.05
        snapMode: Slider.SnapOnRelease
        value: core.script.lastZoomLevel
        width: parent.width * 0.6
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
