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
//import QtQuick.Dialogs 1.0

import Material 0.3

PopupBase {
    id: dialog

    property string src: "avatar.jpg";
    overlayLayer: "dialogOverlayLayer"
    overlayColor: Qt.rgba(0, 0, 0, 0.3)

    opacity: showing ? 1 : 0
    visible: opacity > 0

    width: main.width
    height: main.height

    globalMouseAreaEnabled: false

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

    Behavior on src {
        ScriptAction { script: {canvas.loadImage(dialog.src); canvas.requestPaint()} }
    }

    function show() {
        open()
    }

    Connections{
        target: myProfile
        onFirstIdentityCancelled: {
            mask.enabled = false
            mask.visible = false
        }
        onFirstIdentityCreated: {
            main.owngxs = myProfile.getPreferredGxs()
            //console.log(main.owngxs)
            wallPostModel.setMyWall();
            pageStack.push({item: Qt.resolvedUrl("Content.qml"), immediate: true, replace: true, properties: {name: myProfile.getNickBaseIdentity(), mgxs: myProfile.getPreferredGxs()}})
            main.content.activated = true;
            gridLayout.reorder();
            close()
        }
    }

    MouseArea{
        anchors.fill: parent
        onClicked: {}
    }

    View {
        id: dialogContainer

        width: dp(350)
        height: dp(400)

        anchors {
            centerIn: parent
        }

        elevation: 5
        radius: 2 * Units.dp
        backgroundColor: "white"
        clip: true

        Rectangle{
            id: mask
            anchors.fill: parent
            z: 5
            enabled: false
            visible: false
            color: Qt.rgba(255,255,255,0.8)

            Behavior on visible{
                NumberAnimation { target: mask; property: "opacity"; from: 0; to: 1; duration: MaterialAnimation.pageTransitionDuration }
            }

            MouseArea{
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {}
                onEntered: {}
                onExited: {}
            }

            ProgressCircle {
                id: progressCircle
                anchors.centerIn: parent
                color: Theme.accentColor
                width: dp(48)
                height: dp(48)
                dashThickness: dp(7)
            }
        }

        Canvas {
            id: canvas
            anchors.top: parent.top
            anchors.topMargin: parent.height*0.1
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width < parent.height ? parent.width*0.63 : parent.height*0.63
            height: parent.width < parent.height ? parent.width*0.63 : parent.height*0.63

            Component.onCompleted:loadImage(dialog.src);//("image://avatar/"+"0");

            onPaint: {
                var ctx = getContext("2d");
                if (canvas.isImageLoaded(dialog.src)) {//("image://avatar/"+"0")) {
                    var profile = Qt.createQmlObject('import QtQuick 2.5; Image{source: dialog.src;  visible:false}', canvas);
                    var centreX = width/2;
                    var centreY = height/2;

                    ctx.save();
                        ctx.beginPath();
                            ctx.arc(centreX, centreY, width / 2, 0, Math.PI*2, true);
                        ctx.closePath();
                        ctx.clip();
                        ctx.drawImage(profile, 0, 0, canvas.width, canvas.height);
                    ctx.restore();

                }
            }

            onImageLoaded:requestPaint()

            Ink {
                id: circleInk
                anchors.fill: parent
                circular:true

                Rectangle{
                    anchors.fill: parent
                    color: "black"
                    opacity: circleInk.containsMouse ? 0.1 : 0
                    radius: width/2
                }
                Icon {
                    anchors.centerIn: parent
                    name: "awesome/upload"
                    color: "white"
                    size: parent.width/3
                    opacity: circleInk.containsMouse ? 0.9 : 0
                }
/*
                FileDialog {
                    id: fileDialog
                    title: "Please choose an avatar"
                    folder: shortcuts.pictures
                    selectMultiple: false
                    onAccepted: {
                        dialog.src = fileDialog.fileUrl
                        canvas.loadImage(dialog.src)
                    }
                }
                onClicked: fileDialog.open()*/
            }
        }

        TextField {
            id: name
            anchors.top: canvas.bottom
            anchors.topMargin: dp(30)
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width < parent.height ? parent.width*0.63 : parent.height*0.63
            color: Theme.primaryColor

            placeholderHorizontalCenter: true
            horizontalAlignment: TextInput.AlignHCenter
            placeholderText: "Joe Smith"
            placeholderPixelSize: dp(18)

            font {
                family: "Roboto"
                pixelSize: 18 * Units.dp
                capitalization: Font.MixedCase
            }

            onAccepted: {
                mask.enabled = true
                mask.visible = true
                myProfile.addIdentity(name.text, dialog.src)
            }
        }

        Button {
            id: positiveButton

            text: "CREATE IDENTITY"
            textColor: Theme.accentColor
            context: "dialog"
            size: dp(15)

            anchors {
                horizontalCenter: parent.horizontalCenter
                bottomMargin: 25 * Units.dp
                bottom: parent.bottom
            }

            onClicked: {
                mask.enabled = true
                mask.visible = true
                myProfile.addIdentity(name.text, dialog.src)
            }
        }
    }
}
