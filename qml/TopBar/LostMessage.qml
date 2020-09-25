import QtQuick 2.0
import StyleConfig 1.0

Text {
    id: root

    text: "You lost!"
    color: StyleConfig.gameStatusMessageColor

    anchors {
        verticalCenter: parent.verticalCenter
        left: parent.left
        leftMargin: parent.width * StyleConfig.gameStatusMessageMarginKoefficient
    }

    font {
        pixelSize: Math.min(parent.height, parent.width) * StyleConfig.gameStatusMessageSizeKoefficient
        bold: true
    }
}
