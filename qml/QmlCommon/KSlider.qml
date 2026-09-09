import QtQuick 2.15
import QtQuick.Controls 2.15

Slider {
    id: control

    implicitWidth: 200
    implicitHeight: Math.max(knobSize, Math.round(44 * AppPalette.scale))
    horizontalPadding: knobSize / 2
    verticalPadding: 0
    snapMode: Slider.SnapAlways
    opacity: enabled ? 1.0 : 0.55
    focusPolicy: Qt.StrongFocus

    property color trackOffColor:         AppPalette.trackOff
    property color trackOffBorderColor:   AppPalette.trackOffBorder
    property color trackFillColor:        AppPalette.accentBar
    property color knobColor:             AppPalette.knob
    property color knobBorderColor:       AppPalette.borderHover
    property color knobBorderActiveColor: AppPalette.accentBorder

    property int   trackHeight: Math.round(8  * AppPalette.scale)
    property int   knobSize:    Math.round(25 * AppPalette.scale)

    property string toolTipText:   ""
    property bool   showValueTip:  true
    property int    valueDecimals: 0
    property real   valueDivisor:  1.0
    property string valueSuffix:   ""
    property real   _grabOffset:   0.0

    signal valueModified(real val)

    function _valueAtX(x) {
        let p = (x - control.leftPadding) / Math.max(control.availableWidth, 1.0)
        p = Math.max(0.0, Math.min(1.0, p))
        if (control.mirrored) {
            p = 1.0 - p
        }
        let v = control.from + p * (control.to - control.from)
        if (control.stepSize > 0) {
            v = control.from + Math.round((v - control.from) / control.stepSize) * control.stepSize
        }
        return Math.max(control.from, Math.min(control.to, v))
    }

    function _isOnHandle(x, y) {
        const h = control.handle
        if (!h) {
            return false
        }
        return x >= h.x && x <= h.x + h.width && y >= h.y && y <= h.y + h.height
    }

    onMoved: control.valueModified(value)

    Keys.onLeftPressed:  function(e) { control.decrease(); control.valueModified(control.value); e.accepted = true }
    Keys.onDownPressed:  function(e) { control.decrease(); control.valueModified(control.value); e.accepted = true }
    Keys.onRightPressed: function(e) { control.increase(); control.valueModified(control.value); e.accepted = true }
    Keys.onUpPressed:    function(e) { control.increase(); control.valueModified(control.value); e.accepted = true }

    background: Rectangle {
        x: control.leftPadding
        y: control.topPadding + (control.availableHeight - height) / 2
        width: control.availableWidth
        height: control.trackHeight
        radius: height / 2
        color: control.trackOffColor
        border.width: Tokens.cardBorderWidth
        border.color: control.trackOffBorderColor

        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            radius: parent.radius
            color: control.trackFillColor

            Behavior on width {
                enabled: !inputArea.pressed
                NumberAnimation { duration: 100; easing.type: Easing.OutCubic }
            }
        }
    }

    handle: Rectangle {
        x: control.leftPadding + control.visualPosition * control.availableWidth - width / 2
        y: control.topPadding + (control.availableHeight - height) / 2
        width: control.knobSize * 2
        height: control.knobSize
        radius: width  * 0.5
        color: "#f0f8ff"
        border.width: inputArea.pressed || inputArea.containsMouse || control.visualFocus ? 2 : 1
        border.color: inputArea.pressed || inputArea.containsMouse || control.visualFocus
                      ? control.knobBorderActiveColor : control.knobBorderColor

        Behavior on border.color {
            ColorAnimation { duration: 100 }
        }
    }


    MouseArea {
        id: inputArea
        anchors.fill: parent
        hoverEnabled: true
        preventStealing: true

        onPressed: {
            control.forceActiveFocus()
            if (control._isOnHandle(mouse.x, mouse.y)) {
                // 点击在手柄上：只记抓取偏移，值不变——未拖动则滑块不动
                control._grabOffset = mouse.x - (control.handle.x + control.handle.width / 2)
            }
            else {
                // 点击在轨道上：立即跳到点击处，随后拖动跟随光标
                control._grabOffset = 0
                const v = control._valueAtX(mouse.x)
                if (v !== control.value) {
                    control.value = v
                }
                control.valueModified(control.value)
            }
        }

        onPositionChanged: {
            if (!pressed) {
                return
            }
            const v = control._valueAtX(mouse.x - control._grabOffset)
            if (v !== control.value) {
                control.value = v
                control.valueModified(v)
            }
        }
    }



    KToolTip {
        targetItem: control
        // shown: control.showValueTip && (control.pressed || control.hovered) && control.enabled
        shown: false
        text: control.toolTipText.length > 0
              ? control.toolTipText
              : (control.value / control.valueDivisor).toFixed(control.valueDecimals) + control.valueSuffix
    }
}
