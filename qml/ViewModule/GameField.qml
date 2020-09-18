import QtQuick 2.0
import GameFieldModel 1.0;
import StyleConfig 1.0

Item {
    id: root

    function gameFieldReset() {
        gameModel.gameFieldReset();
    }

    GridView {
        id: grid

        property int ballRadius: Math.min(root.height / gameModel.gameFieldHeight, root.width / gameModel.gameFieldWidth)

        anchors {
            centerIn: root
        }

        width: ballRadius * gameModel.gameFieldWidth
        height: root.height

        cellHeight: ballRadius
        cellWidth: ballRadius

        interactive: false

        model: GameModel {
            id: gameModel

        }

        delegate: BallDelegate {
            id: ballDelegate

            width: grid.cellWidth
            height: grid.cellHeight

            displayedСolor: itemColor

            onClicked: {
                gameModel.selectItem(index)
            }

            selected: isSelected ?
                          true :
                          false
        }

        moveDisplaced: Transition {
            NumberAnimation {
                properties: "x"
                duration: StyleConfig.ballMoveAnimationDuration;
            }
        }

        move: Transition {
            NumberAnimation {
                properties: "x, y"
                duration: StyleConfig.ballMoveAnimationDuration;
            }

        }
    }
}
