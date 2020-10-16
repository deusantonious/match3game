import QtQuick 2.15
import GameFieldModel 1.0;
import StyleConfig 1.0

Item {
    id: root

    signal gameIsLost
    signal gameIsWon

    function gameFieldReset() {
        gameModel.gameFieldReset();
    }

    GridView {
        id: grid

        property int ballRadius: Math.min(root.height / gameModel.currentRowCount, root.width / gameModel.currentColumnCount)

        anchors {
            centerIn: root
        }

        width: ballRadius * gameModel.currentColumnCount
        height: ballRadius * gameModel.currentRowCount

        cellHeight: ballRadius
        cellWidth: ballRadius

        interactive: false

        model: GameModel {
            id: gameModel

            onGameLostChanged: {
                gameIsLost();
            }
            onGameWonChanged: {
                gameIsWon();
            }
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
                gameModel.deleteAllEmptyRowsAndColumns();

                if (gameModel.gameIsWon()) {
                    gameIsWon();
                }
                else if (gameModel.gameIsLost()) {
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
            NumberAnimation {
                properties: "x, y"
                duration: StyleConfig.ballMoveAnimationDuration;
            }

            onRunningChanged: {
                if (!running) {
                    gameModel.makeAllCoincidenceInvisible();
                }
            }
        }
    }
}
