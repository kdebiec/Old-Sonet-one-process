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
import QtQuick.Layouts 1.3

import Material 0.3
import Material.Extras 0.1 as Circle
import Material.ListItems 0.1 as ListItem

Component{
    id: notificationDelegate

    Rectangle{
        id: friendroot
        property bool entered: false
        property string msg: statusmsg

        clip: true
        width: parent.width
        height: 50

        transitions:[
            Transition {
                from: "hidden"; to: "entered"

                ParallelAnimation {
                    NumberAnimation { target: icons; property: "y"; from: 40; to: 0; duration: MaterialAnimation.pageTransitionDuration }
                    NumberAnimation { target: icons; property: "opacity"; from: 0; to: 1; duration: MaterialAnimation.pageTransitionDuration }

                    NumberAnimation { target: text; property: "y"; from: 0; to: -40; duration: MaterialAnimation.pageTransitionDuration }
                    NumberAnimation { target: text; property: "opacity"; from: 1; to: 0; duration: MaterialAnimation.pageTransitionDuration }
                }
            },
            Transition {
                from: "entered"; to: "hidden"

                ParallelAnimation {
                    NumberAnimation { target: text; property: "y"; from: -40; to: 0; duration: MaterialAnimation.pageTransitionDuration }
                    NumberAnimation { target: text; property: "opacity"; from: 0; to: 1; duration: MaterialAnimation.pageTransitionDuration }

                    NumberAnimation { target: icons; property: "y"; from: 0; to: 40; duration: MaterialAnimation.pageTransitionDuration }
                    NumberAnimation { target: icons; property: "opacity"; from: 1; to: 0; duration: MaterialAnimation.pageTransitionDuration }
                }
            }
        ]


        states:[
            State {
                name: "hidden"; when: entered === false
                PropertyChanges { target: friendroot; color: "#ffffff" }
            },
            State {
                name: "entered"; when: entered === true
                PropertyChanges { target: friendroot; color: Qt.rgba(0,0,0,0.03) }
            }
        ]

        MouseArea{
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            hoverEnabled: true
            onEntered: friendroot.entered = true;
            onExited: friendroot.entered = false;
            onClicked: overflowMenu.open(friendroot, mouse.x, mouse.y);

            states:[
                State {
                    name: "name"; when: msg === ""
                    PropertyChanges { target: name; height: friendroot.height; verticalAlignment: Text.AlignVCenter; }
                },
                State {
                    name: "smsg"; when: msg != ""
                    PropertyChanges { target: name; height: friendroot.height/2; verticalAlignment: Text.AlignBottom; }
                }
            ]

            Dropdown {
                id: overflowMenu
                objectName: "overflowMenu"
                overlayLayer: "dialogOverlayLayer"
                width: 200 * Units.dp
                height: dp(4*30)
                enabled: true
                anchor: Item.TopLeft
                durationSlow: 200
                durationFast: 100

                Column{
                    anchors.fill: parent
                    ListItem.Standard {
                        height: dp(30)
                        text: "Profile"
                        itemLabel.style: "menu"
                        onClicked: {
                            overflowMenu.close()
                            wallPostModel.loadWall(gxsid);
                            pageStack.push({item: Qt.resolvedUrl("Content.qml"), immediate: true, replace: true, properties: {name: friendname, mgxs: gxsid, statusgxs: Qt.binding(function(){return statuscolor})}})
                            main.content.activated = true;
                        }
                    }

                    ListItem.Standard {
                        height: dp(30)
                        text: "Chat"
                        itemLabel.style: "menu"
                        onClicked: {
                            overflowMenu.close()
                            msgStore.cleanStoredMsgForPGP(rspgpid)
                            main.createChatCard(friendname, rspgpid, "ChatCard.qml", gxsid)
                        }
                    }

                    ListItem.Standard {
                        height: dp(30)
                        text: "Call"
                        itemLabel.style: "menu"
                        onClicked: {
                            overflowMenu.close()
                            main.createCallCard(gxsid, friendname, "CallCard.qml")
                        }
                    }

                    ListItem.Standard {
                        height: dp(30)
                        text: "Remove"
                        itemLabel.style: "menu"
                        onClicked: {
                            overflowMenu.close()
                            friendList.removeGxsContact(gxsid);
                        }
                    }
                }
            }

            Canvas {
                id: canvas
                anchors.verticalCenter: parent.verticalCenter
                x: 10
                width: 40
                height: 40
                Component.onCompleted:loadImage("image://avatar/"+"gxs/"+gxsid);
                onPaint: {
                    var ctx = getContext("2d");
                    if (canvas.isImageLoaded("image://avatar/"+"gxs/"+gxsid)) {
                        var profile = Qt.createQmlObject('import QtQuick 2.5; Image{source: "image://avatar/"+"gxs/"+gxsid;  visible:false}', canvas);
                        var centreX = width/2;
                        var centreY = height/2;

                        ctx.beginPath();
                        ctx.moveTo(centreX, centreY);
                        ctx.arc(centreX, centreY, width / 2, 0, Math.PI * 2, false);
                        ctx.clip();
                        ctx.drawImage(profile, 0, 0, canvas.width, canvas.height);
                    }
                }
                onImageLoaded:requestPaint()

                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        wallPostModel.loadWall(gxsid);
                        pageStack.push({item: Qt.resolvedUrl("Content.qml"), immediate: true, replace: true, properties: {name: friendname, mgxs: gxsid, statusgxs: Qt.binding(function(){return statuscolor})}})
                        main.content.activated = true;
                    }
                }


            }

            Item{
                id: text

                x: 60
                width: 151
                height: parent.height

                Text {
                    id: name
                    height: parent.height
                    text: friendname
                    color: Theme.light.textColor
                    font.family: "Roboto"
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                    font.pixelSize: 14
                }
                Text {
                    id: smsg
                    y: parent.height/2
                    height: parent.height/2
                    color: statuscolor
                    text: msg
                    font.family: "Roboto"
                    verticalAlignment: Text.AlignTop
                    horizontalAlignment: Text.AlignLeft
                    font.pixelSize: 12
                }
            }

            Item{
                id: icons

                height: parent.height
                x: 60
                y: 50

                Icon {
                    id: circle1

                    anchors.verticalCenter: parent.verticalCenter
                    height: parent.height

                    name: "awesome/comment"
                    visible: true
                    color: Theme.light.iconColor

                    size: 31 * Units.dp

                    View{
                        anchors.top: circle1.top
                        anchors.right: circle1.right
                        anchors.topMargin: dp(10)

                        width: dp(14)
                        height: dp(14)
                        radius: width/2
                        backgroundColor: statuscolor
                        elevation: 1

                        visible: msgcount > 0 ? true : false

                        Text{
                            anchors.fill: parent
                            text: msgcount
                            color: "white"
                            font.family: "Roboto"
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }

                    MouseArea{
                        anchors.fill: parent
                        onClicked: {
                            msgStore.cleanStoredMsgForPGP(rspgpid)
                            main.createChatCard(friendname, rspgpid, "ChatCard.qml", gxsid)
                        }
                    }
                }

                Icon {
                    id: circle2

                    anchors.verticalCenter: parent.verticalCenter
                    x: 40
                    height: parent.height

                    name: "awesome/phone"
                    visible: true
                    color: Theme.light.iconColor

                    size: 31 * Units.dp

                    MouseArea{
                        anchors.fill: parent
                        onClicked: main.createCallCard(gxsid, friendname, "CallCard.qml")
                    }
                }

                Icon {
                    id: circle3

                    anchors.verticalCenter: parent.verticalCenter
                    x: 80
                    height: parent.height

                    name: "awesome/video_camera"
                    visible: true
                    color: Theme.light.hintColor

                    size: 31 * Units.dp

                    MouseArea{
                        anchors.fill: parent
                        onClicked: {}
                    }
                }
            }

            View{
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: msgcount > 0 ? 10 : 15
                width: msgcount > 0 ? 20 : 10
                height: msgcount > 0 ? 20 : 10
                radius: width/2
                backgroundColor: statuscolor
                elevation: msgcount > 0 ? 1 : 0

                Text{
                    anchors.fill: parent
                    text: msgcount
                    color: "white"
                    font.family: "Roboto"
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    visible: msgcount > 0 ? true : false
                }
            }
        }
    }
}
