import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import RobotControl 1.0
import "components"

ApplicationWindow  {
    id: app
    width: 640
    height: 480
    visible: true
    title: qsTr("Robot arm control")

    property real buttonHeight: 30
    property real margins: 10

    Engine {
        id: engine
    }

    ColumnLayout {
        anchors.fill: parent

        RobotStatusBar {
            id: statusBar
            Layout.fillWidth: true
            Layout.preferredHeight: app.buttonHeight
        }

        MainView {
            id: mainView
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
