import QtQuick 2.15
import StyleConfig 1.0

MouseArea {
    id: root

    property string displayedСolor: {""}
    property bool selected: false

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

        Behavior on scale {
            ScaleAnimator {
                target: ball;
                duration: StyleConfig.ballScaleaAnimationDuration
            }
        }
    }
}
