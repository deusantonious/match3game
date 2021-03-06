pragma Singleton

import QtQuick 2.0

QtObject {
    readonly property string topBarColor: "#d7d2cc"
    readonly property double topBarHeightKoefficient: 0.1

    readonly property string resetTextColor: "#a08c7d"
    readonly property double resetHeightKoefficient: 0.7
    readonly property double resetWidthKoefficient: 0.2

    readonly property double resetButtonWidthKoefficient: 0.2
    readonly property double resetButtonBrightnessContainsMouse: -0.1

    readonly property double ballMarginsKoefficient: 0.05

    readonly property double ballScaleIfSelected: 0.7
    readonly property double ballScaleaAnimationDuration: 100
    readonly property double ballMoveAnimationDuration: 300
    readonly property double ballDeleteAnimationDuration: 300

    readonly property double gameStatusMessageMarginKoefficient: 0.02
    readonly property double gameStatusMessageSizeKoefficient: 0.75
    readonly property string gameStatusMessageColor: "#a08c7d"

    readonly property double shakeAnimationDuration: 50
    readonly property double shakeAnimationCount: 3
    readonly property double shakeAnimationScaleValue: 1.1

}
