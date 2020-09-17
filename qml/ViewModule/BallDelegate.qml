import QtQuick 2.0
import StyleConfig 1.0

MouseArea {
    id: root

    Rectangle {
        id: ball

        anchors {
            margins: root.height * StyleConfig.ballMarginsKoefficient // heigh always equal width
            fill: parent
        }

        color: itemColor

        radius: root.height
    }
}
