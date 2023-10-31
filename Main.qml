import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Controls.Material

import "ui"

ApplicationWindow  {
    id: app
    width: 640
    height: 480
    visible: true
    title: qsTr("Robot arm control")

    property real buttonHeight: 25

    MainView {
        id: mainView
        anchors.fill: parent

    }

}
