import QtQuick
import QtQuick.Controls
import voidtweak

Item {
    id: control
    implicitHeight: entriesColumn.height

    property EntityEntry entry
    property bool hasEntries: !!control.entry.entries.length
    property int depth: 1

    Column {
        id: entriesColumn
        width: parent.width

        Label {
            text: {
                if (control.entry.isKiscule) {
                    return `${control.entry.key} = SPECIAL KISCULE NODE HANDLING COMING SOON!;`
                } else if (control.hasEntries) {
                    return `${control.entry.key} = {`
                } else {
                    return `${control.entry.key} = ${control.entry.value};`
                }
            }
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

    Component.onCompleted: {
        if (control.entry.isKiscule) {
            return
        }
        var comp = Qt.createComponent("EntityEntryListItem.qml")
        if (comp.status === Component.Ready) {
            for (var i = 0; i < control.entry.entries.length; i++) {
                comp.createObject(entriesColumn, {
                                      "x": 20,
                                      "width": entriesColumn.width - 20,
                                      "depth": control.depth + 1,
                                      "entry": control.entry.entries[i]
                                  })
            }
            if (hasEntries) {
                endLabel.createObject(entriesColumn, {
                                          "width": entriesColumn.width
                                      })
            }
        }
    }
}
