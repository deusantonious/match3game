import QtQuick 2.15
import GameFieldModel 1.0;
import StyleConfig 1.0

Item {
    id: root

    signal gameIsLost();

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

            isVisible: ballIsVisible
            selected: isSelected

            onClicked: {
                if (gameModel.selectedItemId === -1 || !gameModel.selectedItemBordersWith(index)) {
                    gameModel.selectItem(index);
                }
                else {
                    if (!gameModel.swapSelectedItemWith(index)) {
                        enableShakeAnimation();
                    }
                }
            }

            onItemDeleted: {
               gameModel.moveToFloor();

                if (gameModel.gameIsLost()) {
                    gameIsLost();
                }
            }
        }

        moveDisplaced: Transition {
            NumberAnimation {
                properties: "x"
                duration: StyleConfig.ballMoveAnimationDuration;
            }
        }

        move: Transition {
            onRunningChanged: {
                if(!running) {
                    gameModel.makeAllCoincidenceInvisible()
                }
            }

            NumberAnimation {
                properties: "x, y"
                duration: StyleConfig.ballMoveAnimationDuration;
            }
        }
    }
}
