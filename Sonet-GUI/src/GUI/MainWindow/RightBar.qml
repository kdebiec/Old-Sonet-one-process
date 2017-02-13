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

import QtQuick 2.5
import Material 0.3

View {
    id: rightBar

    property string color: "white"

    anchors {
        top: parent.top
        right: parent.right
        bottom: parent.bottom
        topMargin: dp(50)
    }

    width: dp(210)

    backgroundColor: Theme.tabHighlightColor
    elevation: 2

    clipContent: true

    Item{
        id: gxsBox
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: dp(3)
        state: "bigGxsBox"

        states: [
            State {
                name: "smallGxsBox"
                PropertyChanges {
                    target: gxsBox
                    anchors.bottomMargin: (parent.height/2)-dp(25)
                }
            },
            State {
                name: "bigGxsBox"
                PropertyChanges {
                    target: gxsBox
                    anchors.bottomMargin: dp(50)
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { target: gxsBox; property: "anchors.bottomMargin"; duration: MaterialAnimation.pageTransitionDuration/2 }
            }
        ]

        Item{
            height: dp(50)
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            TextField {
                id: statusm
                placeholderText: "Set your status msg"
                anchors.leftMargin: dp(10)
                anchors.rightMargin: dp(50)
                anchors.topMargin: dp(10)
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: parent.right

                text: myProfile.getStatusMsg()

                font {
                    family: "Roboto"
                    pixelSize: 16 * Units.dp
                    capitalization: Font.MixedCase
                }

                showBorder: false

                onAccepted: {
                    myProfile.setStatusMsg(statusm.text)
                    statusm.text = myProfile.getStatusMsg()
                }
            }

            MouseArea{
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                width: dp(50)
                height: parent.height
                hoverEnabled: true
                onEntered: changeStatus.state = "big"
                onExited: changeStatus.state = "small"

                onClicked: {
                    myProfile.changeStatus();
                    changeStatus.backgroundColor = myProfile.getStatus();
                }
            }

            View {
                id: changeStatus
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: dp(15)
                width: dp(10)
                height: dp(10)
                radius: width/2
                elevation: 3/2
                backgroundColor: myProfile.getStatus()

                states: [
                    State {
                        name: "small"
                        PropertyChanges {
                            target: changeStatus
                            width: dp(10)
                            height: dp(10)
                            anchors.rightMargin: dp(15)
                        }
                    },
                    State {
                        name: "big"
                        PropertyChanges {
                            target: changeStatus
                            width: dp(24)
                            height: dp(24)
                            anchors.rightMargin: dp(8)
                        }
                    }
                ]

                transitions: [
                    Transition {
                        ParallelAnimation {
                            NumberAnimation { target: changeStatus; property: "height"; duration: MaterialAnimation.pageTransitionDuration/3 }
                            NumberAnimation { target: changeStatus; property: "width"; duration: MaterialAnimation.pageTransitionDuration/3 }
                            NumberAnimation { target: changeStatus; property: "anchors.rightMargin"; duration: MaterialAnimation.pageTransitionDuration/3 }
                        }
                    }
                ]

                MouseArea{
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: changeStatus.state = "big"
                    onExited: changeStatus.state = "small"

                    onClicked: {
                        myProfile.changeStatus();
                        changeStatus.backgroundColor = myProfile.getStatus();
                    }
                }
            }
        }

        ListView {
            id: listView
            anchors.fill: parent
            anchors.topMargin: dp(50)

            clip: true

            model: gxsIdModel
            delegate: FriendListDelegate{}
        }
        Scrollbar {
            flickableItem: listView
        }
    }

    Item{
        id: pgpBox
        anchors.left:parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: dp(50)

        states: [
            State{
                name: "notvisible"; when: !main.advmode
                PropertyChanges {
                    target: pgpBox
                    visible: false
                }
            },
            State {
                name: "visible"; when: !main.advmode
                PropertyChanges {
                    target: pgpBox
                    visible: true
                }
            },
            State {
                name: "smallPgpBox"
                PropertyChanges {
                    target: pgpBox
                    height: dp(50)
                }
            },
            State {
                name: "bigPgpBox"
                PropertyChanges {
                    target: pgpBox
                    height: parent.height/2
                }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation { target: pgpBox; property: "height"; duration: MaterialAnimation.pageTransitionDuration/2 }
            }
        ]

        Button {
            anchors.left:parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: dp(50)
            size: 13
            text: "PgpBox"
            textColor: "white"
            backgroundColor: Palette.colors["deepOrange"]["500"]
            elevation: 1

            onClicked: {
                gxsBox.state === "smallGxsBox" ? gxsBox.state = "bigGxsBox" : gxsBox.state = "smallGxsBox"
                pgpBox.state === "bigPgpBox" ? pgpBox.state = "smallPgpBox" : pgpBox.state = "bigPgpBox"
            }

            Icon {
                id: pgpBoxIcon

                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: height

                states: [
                    State {
                        name: "nonrotated"; when: pgpBox.height !== dp(50)
                        PropertyChanges {
                            target: pgpBoxIcon
                            rotation: 0
                        }
                    },
                    State {
                        name: "rotated"; when: pgpBox.height === dp(50)
                        PropertyChanges {
                            target: pgpBoxIcon
                            rotation: 90
                        }
                    }
                ]

                name: "awesome/chevron_down"
                color: "white"

                size: 20 * Units.dp

                Behavior on rotation {
                    NumberAnimation { duration: MaterialAnimation.pageTransitionDuration/2 }
                }
            }
        }

        ListView {
            id: listView2
            anchors.fill: parent
            anchors.topMargin: dp(50)
            anchors.rightMargin: dp(3)

            clip: true

            model: pgpListModel
            delegate: PgpListDelegate{}
        }

        Scrollbar {
            flickableItem: listView2
        }
    }

    ParallelAnimation {
        running: true
        NumberAnimation { target: rightBar; property: "anchors.rightMargin"; from: -50; to: 0; duration: MaterialAnimation.pageTransitionDuration }
        NumberAnimation { target: rightBar; property: "opacity"; from: 0; to: 1; duration: MaterialAnimation.pageTransitionDuration }
    }
}
