import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import RobotControl 1.0


Item {
    id: root

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: app.margins
        anchors.rightMargin: app.margins

        Rectangle {
            Layout.preferredHeight: root.height * 0.8
            Layout.preferredWidth: root.height * 0.8
            Layout.alignment: Qt.AlignVCenter
            radius: height / 2
            color: {
                var ledColor = "white";
                switch(engine.robotController.state) {
                case RobotController.StateReady:
                    ledColor = "green";
                    break;
                case RobotController.StateError:
                    ledColor = "red";
                    break;
                case RobotController.StateDisconnected:
                    ledColor = "blue";
                    break;
                case RobotController.StateInitializing:
                    ledColor = "orange";
                    break;
                }

                return ledColor
            }
        }

        Item {
            Layout.preferredHeight: root.height
            Layout.preferredWidth: root.height
            BusyIndicator {
                anchors.fill: parent
                anchors.margins: root.height * 0.2
                running: engine.robotController.state === RobotController.StateInitializing
            }

        }



        Label {
            Layout.alignment: Qt.AlignVCenter
            text: engine.robotController.firmwareVersion
        }

        Item {
            Layout.fillWidth: true
        }


    }
}
