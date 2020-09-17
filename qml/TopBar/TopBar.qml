import QtQuick 2.0
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12
import StyleConfig 1.0

Rectangle {
    id: root

    signal resetButtonClicked

    color: StyleConfig.topBarColor

    Rectangle {
        id: resetButton

        anchors {
            verticalCenter: root.verticalCenter
            right: root.right
        }

        color: StyleConfig.topBarColor

        height: parent.height
        width: parent.width * StyleConfig.resetButtonWidthKoefficient


        Text {
            id: resetTextField

            anchors {
                centerIn: resetButton
            }
            color: StyleConfig.resetTextColor

            text: "reset"

            font {
                pixelSize: resetButton.width > 5 * resetButton.height * StyleConfig.resetHeightKoefficient ?
                               resetButton.height * StyleConfig.resetHeightKoefficient :
                               resetButton.width * StyleConfig.resetWidthKoefficient
                bold: true
            }

        }

        MouseArea {
            id: buttonMouseArea

            anchors.fill: resetButton
            hoverEnabled: true

            onClicked: {
                root.resetButtonClicked();
            }
        }

        BrightnessContrast {
            id: brightnessModifier

            anchors.fill: resetButton
            source: resetButton

            brightness: buttonMouseArea.containsMouse ?
                            StyleConfig.resetButtonBrightnessContainsMouse
                          : 0
        }
    }
}
