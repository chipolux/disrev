import QtQuick
import QtQuick.Controls

Item {
    Rectangle {
        id: entitiesTreeBackground
        color: "#444"
        radius: 4
        anchors.top: filterInput.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 5
    }

    TreeView {
        id: entitiesTree
        model: core.entitiesProxy
        boundsBehavior: TreeView.StopAtBounds
        anchors.fill: entitiesTreeBackground
        anchors.margins: 5

        delegate: Item {
            id: treeDelegate
            implicitWidth: treeView.width
            implicitHeight: label.implicitHeight * 1.6

            readonly property real indent: 15
            readonly property real padding: 0
            required property TreeView treeView
            required property bool isTreeNode
            required property bool expanded
            required property int hasChildren
            required property int depth

            TapHandler {
                onTapped: treeView.toggleExpanded(row)
            }

            Text {
                id: indicator
                x: treeDelegate.padding + (treeDelegate.depth * treeDelegate.indent)
                visible: treeDelegate.isTreeNode && treeDelegate.hasChildren
                text: treeDelegate.expanded ? "▼" : "▶"
                color: "#DDD"
                anchors.verticalCenter: label.verticalCenter
            }

            Text {
                id: label
                x: treeDelegate.padding
                   + (treeDelegate.isTreeNode ? (treeDelegate.depth + 1) * treeDelegate.indent : 0)
                width: treeDelegate.width - treeDelegate.padding - x
                text: model.display
                color: "#DDD"
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

        onClicked: core.clearEntities()
    }

    TextField {
        id: filterInput
        placeholderText: "Filter entities..."
        anchors.left: clearButton.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 5

        onEditingFinished: core.entitiesProxy.setFilterFixedString(text)
    }
}
