import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

ApplicationWindow {
    id: applicationWindow1
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")

    Label {
        text: qsTr("Nothing here, check the <b>adb logcat</b> output")
        anchors.centerIn: parent
    }
}
