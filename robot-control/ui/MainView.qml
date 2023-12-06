import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import "components"

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent

        TabBar {
            id: tabBar
            Layout.fillWidth: true
            Layout.preferredHeight: app.buttonHeight

            TabButton {
                height: app.buttonHeight
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Home")
            }

            TabButton {
                height: app.buttonHeight
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Settings")
            }
        }

        StackLayout {
            id: stackLayout
            Layout.fillWidth: true
            Layout.fillHeight: true

            currentIndex: tabBar.currentIndex

            HomeView {
                id: homeTab
            }

            SettingsView {
                id: discoverTab
            }
        }
    }
}

