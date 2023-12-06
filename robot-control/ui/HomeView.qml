import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import RobotControl 1.0

Item {
    id: root

    ColumnLayout {
        anchors.left: parent.left
        anchors.top: parent.top
        width: parent.width / 2
        anchors.margins: app.margins

        RowLayout {
            Layout.fillWidth: true

            Label {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                text: qsTr("Stepper enabled")
            }

            Switch {
                id: steppersEnabledSwitch
                Layout.alignment: Qt.AlignVCenter
                checkable: engine.robotController.state === RobotController.StateReady
                checked: engine.robotController.steppersEnabled
                onCheckedChanged: engine.robotController.steppersEnabled = checked
                Component.onCompleted: checked = engine.robotController.steppersEnabled
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                text: qsTr("Move to home position")
            }

            Button {
                id: homePositionButton
                Layout.alignment: Qt.AlignVCenter
                text: qsTr("Home position")
                enabled: engine.robotController.state === RobotController.StateReady
                onClicked: console.log("Move home position")

            }
        }
    }
}
