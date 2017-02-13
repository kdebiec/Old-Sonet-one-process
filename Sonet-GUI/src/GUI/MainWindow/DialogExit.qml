/****************************************************************
 *  This file is part of Sonet.
 *  Sonet is distributed under the following license:
 *
 *  Copyright (C) 2017, Konrad Dębiec
 *
 *  Sonet is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 3
 *  of the License, or (at your option) any later version.
 *
 *  Sonet is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

import QtQuick 2.4
import QtQuick.Layouts 1.1

import Material 0.3
import Material.Extras 0.1

/*!
   \qmltype Dialog
   \inqmlmodule Material
   \brief Dialogs inform users about critical information, require users to make
   decisions, or encapsulate multiple tasks within a discrete process
 */
PopupBase {
    id: dialog

    overlayLayer: "dialogOverlayLayer"
    overlayColor: Qt.rgba(0, 0, 0, 0.3)

    opacity: showing ? 1 : 0
    visible: opacity > 0

    width: Math.max(minimumWidth,
                    content.contentWidth + 2 * contentMargins)

    height: Math.min(parent.height - 64 * Units.dp,
                     headerView.height +
                     content.contentHeight +
                     (floatingActions ? 0 : buttonContainer.height))

    property int contentMargins: 24 * Units.dp

    property int minimumWidth: Device.isMobile ? 200 * Units.dp : 250 * Units.dp

    property alias title: titleLabel.text
    property alias text: textLabel.text

    property alias positiveButtonEnabled: positiveButton.enabled
    property alias negativeButton: negativeButton
    property alias positiveButton: positiveButton

    property string negativeButtonText: "Cancel"
    property string positiveButtonText: "Ok"

    property bool hasActions: true
    property bool floatingActions: false

    default property alias dialogContent: column.data

    signal accepted()
    signal rejected()

    anchors {
        centerIn: parent
        verticalCenterOffset: showing ? 0 : -(dialog.height/3)

        Behavior on verticalCenterOffset {
            NumberAnimation { duration: 200 }
        }
    }

    Behavior on opacity {
        NumberAnimation { duration: 200 }
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_Escape) {
            closeKeyPressed(event)
        }
    }

    Keys.onReleased: {
        if (event.key === Qt.Key_Back) {
            closeKeyPressed(event)
        }
    }

    function closeKeyPressed(event) {
        if (dialog.showing) {
            if (dialog.dismissOnTap) {
                dialog.close()
            }
            event.accepted = true
        }
    }

    function show() {
        open()
    }

    View {
        id: dialogContainer

        z: 2
        anchors.fill: parent
        elevation: 5
        radius: 2 * Units.dp
        backgroundColor: "white"

        MouseArea {
            anchors.fill: parent
            propagateComposedEvents: false

            onClicked: {
                mouse.accepted = false
            }
        }

        Rectangle {
            anchors.fill: content
        }

        Flickable {
            id: content

            contentWidth: column.implicitWidth
            contentHeight: column.height + (column.height > 0 ? contentMargins : 0)
            clip: true

            anchors {
                left: parent.left
                right: parent.right
                top: headerView.bottom
                bottom: floatingActions ? parent.bottom : buttonContainer.top
            }

            interactive: contentHeight > height

            onContentXChanged: {
                if(contentX != 0 && contentWidth <= width)
                    contentX = 0
            }

            onContentYChanged: {
                if(contentY != 0 && contentHeight <= height)
                    contentY = 0
            }

            Column {
                id: column
                anchors {
                    left: parent.left
                    leftMargin: contentMargins
                }

                width: content.width - 2 * contentMargins
                spacing: 8 * Units.dp
            }
        }

        Scrollbar {
            flickableItem: content
        }

        Item {
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }

            height: headerView.height

            View {
                backgroundColor: "white"
                elevation: content.atYBeginning ? 0 : 1
                fullWidth: true
                radius: dialogContainer.radius

                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                }

                height: parent.height
            }
        }


        Column {
            id: headerView

            spacing: 0

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top

                leftMargin: contentMargins
                rightMargin: contentMargins
            }

            Item {
                width: parent.width
                height: contentMargins
                visible: titleLabel.visible || textLabel.visible
            }

            Label {
                id: titleLabel

                width: parent.width
                wrapMode: Text.Wrap
                style: "title"
                visible: title != ""
            }

            Item {
                width: parent.width
                height: 20 * Units.dp
                visible: titleLabel.visible
            }

            Label {
                id: textLabel

                horizontalAlignment: Text.AlignHCenter
                width: parent.width
                wrapMode: Text.Wrap
                style: "dialog"
                color: Theme.light.subTextColor
                visible: text != ""
            }

            Item{
                height: dp(5)
                width: parent.width
                z: 2
                visible: settings.getAutoLogin()

                RowLayout{
                    y: -7
                    anchors.left: parent.left
                    anchors.leftMargin: dp(9)
                    spacing: -10

                    CheckBox {
                        id: checkBox
                        darkBackground: false
                        checked: settings.getAutoLogin()
                        onClicked: settings.setAutoLogin(checkBox.checked)
                    }
                    Label {
                        text: "Automatically log in"
                        color: Theme.light.subTextColor

                        MouseArea{
                            anchors.fill: parent
                            onClicked: {
                              checkBox.checked = !checkBox.checked
                              checkBox.clicked()
                            }
                        }
                    }
                }
            }

            Item {
                width: parent.width
                height: settings.getAutoLogin() ? contentMargins+dp(10) : contentMargins/2
                visible: textLabel.visible
            }
        }

        Item {
            id: buttonContainer

            anchors {
                bottom: parent.bottom
                right: parent.right
                left: parent.left
            }

            height: hasActions ? (settings.getAutoLogin() ? 42 * Units.dp : 52 * Units.dp) : 2 * Units.dp

            View {
                id: buttonView

                height: parent.height
                backgroundColor: floatingActions ? "transparent" : "white"
                elevation: content.atYEnd ? 0 : 1
                fullWidth: true
                radius: dialogContainer.radius
                elevationInverted: true

                anchors {
                    bottom: parent.bottom
                    right: parent.right
                    left: parent.left
                }

                Button {
                    id: negativeButton

                    visible: hasActions
                    text: negativeButtonText
                    textColor: Theme.accentColor
                    context: "dialog"
                    size: dp(13)

                    anchors {
                        verticalCenter: settings.getAutoLogin() ? undefined : parent.verticalCenter
                        top: settings.getAutoLogin() ? parent.top : undefined
                        left: positiveButton.visible ? positiveButton.right : parent.left
                        leftMargin: 4 * Units.dp
                        rightMargin: 8 * Units.dp
                        right: parent.right
                    }

                    onClicked: {
                        close();
                        rejected();
                    }
                }

                Button {
                    id: positiveButton

                    visible: hasActions
                    text: positiveButtonText
                    textColor: Theme.accentColor
                    context: "dialog"
                    size: dp(13)

                    anchors {
                        verticalCenter: settings.getAutoLogin() ? undefined : parent.verticalCenter
                        top: settings.getAutoLogin() ? parent.top : undefined
                        right: parent.horizontalCenter
                        rightMargin: 4 * Units.dp
                        leftMargin: 8 * Units.dp
                        left: parent.left
                    }

                    onClicked: {
                        close()
                        accepted();
                    }
                }
            }
        }
    }
}
