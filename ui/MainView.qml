import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

Item {
    TabBar {
        id: bar
        width: parent.width
        TabButton {
            text: qsTr("Home")
            height: app.buttonHeight
        }
        TabButton {
            text: qsTr("Settings")
            height: app.buttonHeight
        }
    }

    StackLayout {
        width: parent.width
        currentIndex: bar.currentIndex
        Item {
            id: homeTab
        }
        Item {
            id: settingsTab
        }
    }
}
