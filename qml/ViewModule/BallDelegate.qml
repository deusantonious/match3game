import QtQuick 2.15
import StyleConfig 1.0

MouseArea {
    id: root

    property string displayedСolor: {""}
    property bool isVisible: true
    property bool selected: false

    signal itemDeleted();

    function enableShakeAnimation() {
        shakeAnimation.restart();
    }

    Rectangle {
        id: ball

        anchors {
            margins: root.height * StyleConfig.ballMarginsKoefficient // heigh always equal width
            fill: parent
        }

        color: displayedСolor

        radius: root.height

        scale: selected === true ?
                   StyleConfig.ballScaleIfSelected :
                   1

        states: [
            State { when: isVisible;
                PropertyChanges {   target: ball; opacity: 1.0    }
            },
            State { when: !isVisible;
                PropertyChanges {   target: ball; opacity: 0    }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation {
                    property: "opacity";
                    duration: StyleConfig.ballDeleteAnimationDuration
                }

                onRunningChanged: {
                    if (!running) {
                        itemDeleted();
                    }
                }
            }
        ]

        Behavior on scale {
            ScaleAnimator {
                target: ball;
                duration: StyleConfig.ballScaleaAnimationDuration
            }
        }

        SequentialAnimation {
            id: shakeAnimation

            loops: 5

            NumberAnimation {
                target: ball
                property: "scale"
                duration: StyleConfig.shakeAnimationDuration
                from: 1
                to: StyleConfig.shakeAnimationScaleValue
            }
            NumberAnimation {
                target: ball
                property: "scale"
                duration: StyleConfig.shakeAnimationDuration
                from: StyleConfig.shakeAnimationScaleValue
                to: 1
            }
        }
    }
}
