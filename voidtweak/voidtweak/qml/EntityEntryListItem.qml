import QtQuick
import QtQuick.Controls
import voidtweak

Item {
    id: control
    implicitHeight: entriesColumn.height

    signal editRequested(EntityEntry entry)
    property EntityEntry entry

    Component.onCompleted: buildEntries()
    Connections {
        target: control.entry

        function onEntriesChanged() {
            buildEntries()
        }
    }

    function buildEntries() {
        var comp = Qt.createComponent("EntityEntryListItem.qml")
        if (comp.status === Component.Ready) {
            for (var ci = entriesColumn.children.length; ci > 0; ci--) {
                entriesColumn.children[ci - 1].destroy()
            }
            startLabel.createObject(entriesColumn, {
                                        "text": Qt.binding(function () {
                                            var isKiscule = !!control.entry ? control.entry.isKiscule : false
                                            var hasEntries = !!control.entry ? !!control.entry.entries.length : false
                                            var key = !!control.entry ? control.entry.key : "MISSING"
                                            var value = !!control.entry ? control.entry.value : "MISSING"
                                            if (isKiscule) {
                                                return `${key} = SPECIAL KISCULE NODE HANDLING COMING SOON!;`
                                            } else if (hasEntries) {
                                                return `${key} = {`
                                            } else {
                                                return `${key} = ${value};`
                                            }
                                        })
                                    })
            for (var i = 0; i < control.entry.entries.length; i++) {
                var obj = comp.createObject(entriesColumn, {
                                                "x": 20,
                                                "width": Qt.binding(
                                                             function () {
                                                                 return entriesColumn.width - 20
                                                             }),
                                                "entry": control.entry.entries[i]
                                            })
                obj.editRequested.connect(control.editRequested)
            }
            if (!!control.entry && !!control.entry.entries.length) {
                endLabel.createObject(entriesColumn, {
                                          "width": entriesColumn.width
                                      })
            }
        }
    }

    MouseArea {
        anchors.fill: parent

        onClicked: control.editRequested(control.entry)
    }

    Column {
        id: entriesColumn
        width: parent.width
    }

    Component {
        id: startLabel

        Label {
            color: "#DDD"
            elide: Label.ElideRight
            width: parent.width
            height: implicitHeight * 1.5
        }
    }

    Component {
        id: endLabel

        Label {
            text: `}`
            color: "#DDD"
            height: implicitHeight * 1.5
        }
    }
}
