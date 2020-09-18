import QtQuick 2.12
import QtQuick.Window 2.12
import StyleConfig 1.0
import TopBar 1.0
import ViewModule 1.0

Window {
    id: root

    width: 1280
    height: 720

    minimumHeight: 480
    minimumWidth: 720

    visible: true
    title: qsTr("match 3")

    TopBar {
        id: topBar

        width:  root.width
        height: root.height * StyleConfig.topBarHeightKoefficient

        onResetButtonClicked: {
            gameField.gameFieldReset();
        }
    }

    GameField {
        id: gameField

        width: root.width
        height: root.height - topBar.height

        anchors {
            top: topBar.bottom
            left: root.left
        }
    }

}
