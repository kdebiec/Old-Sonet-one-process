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
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

import Material 0.3
import Material.Extras 0.1 as Circle
import Material.ListItems 0.1 as ListItem

Item{
    id: page
    property string title: "profilePage"
    property string mgxs
    property string name
    property color statusgxs

    // Just for "restore" option
    property int tmpCol: 0
    property int tmpRow: 0
    property int tmpGridX: 0   // Numbering starts from 0
    property int tmpGridY: 0   // Numbering starts from 0
    property bool maximized: false
    //

    Connections{
        target: main.content
        onRefresh: {
            updateVisibleRows()
            if(
                main.content.col === (parseInt(gridLayout.width / (50 + gridLayout.columnSpacing))>= 9 ? 9 : parseInt(gridLayout.width / (50 + gridLayout.columnSpacing))) &&
                main.content.row === main.visibleRows &&
                main.content.gridX === Math.floor(((parseInt(gridLayout.width / (50 + gridLayout.columnSpacing)))-main.content.col)/2) &&
                main.content.gridY === 0
                )
            {
                maximized = true
            }
            else
                maximized = false
        }
    }

    View {
        id: chat

        anchors.fill: parent
        elevation: 2
        backgroundColor: Palette.colors["grey"]["50"]

        MouseArea{
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            onClicked: overflowMenu.open(pageStack, mouse.x, mouse.y);

            Dropdown {
                id: overflowMenu
                objectName: "overflowMenu"
                overlayLayer: "dialogOverlayLayer"
                width: 200 * Units.dp
                height: dp(2*30)
                enabled: true
                anchor: Item.TopLeft
                durationSlow: 200
                durationFast: 100

                Column{
                    anchors.fill: parent

                    ListItem.Standard {
                        height: dp(30)
                        text: maximized ? "Restore" : "Maximize"
                        itemLabel.style: "menu"
                        onClicked: {
                            overflowMenu.close()
                            updateVisibleRows()

                            if(!maximized)
                            {
                                page.tmpGridX = main.content.gridX
                                page.tmpGridY = main.content.gridY
                                page.tmpCol = main.content.col
                                page.tmpRow = main.content.row

                                main.content.col = Qt.binding(function() { return parseInt(gridLayout.width / (50 + gridLayout.columnSpacing))>= 9 ? 9 : parseInt(gridLayout.width / (50 + gridLayout.columnSpacing))});
                                main.content.row = Qt.binding(function() { updateVisibleRows(); return main.visibleRows});
                                main.content.gridX = Qt.binding(function() { return Math.floor(((parseInt(gridLayout.width / (50 + gridLayout.columnSpacing)))-main.content.col)/2)});
                                main.content.gridY = 0

                                maximized = true
                            }
                            else if(maximized)
                            {
                                main.content.gridX = page.tmpGridX
                                main.content.gridY = page.tmpGridY
                                main.content.col = page.tmpCol
                                main.content.row = page.tmpRow
                                maximized = false
                            }

                            main.content.refresh()
                        }
                    }

                    ListItem.Standard {
                        height: dp(30)
                        text: "Hide"
                        itemLabel.style: "menu"
                        onClicked: {
                            overflowMenu.close()
                            main.content.activated = false;
                        }
                    }
                }
            }
        }

        Item{
            id: rightSide
            anchors.fill: parent

            Item{
                id: timeline
                anchors.fill: parent
                clip: true

                ListView{
                    id: timeView
                    anchors.fill: parent
                    anchors.margins: dp(20)
                    model: wallPostModel
                    spacing: 20

                    Component.onCompleted: positionViewAtBeginning()

                    property int position: timeView.visibleArea.yPosition*timeView.contentHeight

                    Behavior on position {
                        ScriptAction { script: {console.log(timeView.position)} }
                    }

                    header: Item{
                                height: main.owngxs === mgxs ? dp(250+commentsHeader.height+20) : dp(250)
                                width: parent.width

                                View{
                                    id: profileCard
                                    anchors.top: parent.top
                                    anchors.right: parent.right
                                    anchors.left: parent.left
                                    height: dp(230)

                                    elevation: 1
                                    radius: 10

                                    MouseArea{
                                        id: mouseArea
                                        anchors.fill: parent
                                        hoverEnabled: true
                                    }

                                    Rectangle{
                                        id: bg
                                        anchors.top: parent.top
                                        anchors.left: parent.left
                                        anchors.right: parent.right
                                        height: profileCard.height*0.65
                                        clip: true
                                        radius: 10

                                        Icon {
                                            id: icon
                                            anchors.top: parent.top
                                            anchors.topMargin: dp(10)
                                            anchors.right: parent.right
                                            anchors.rightMargin: dp(10)
                                            name: "awesome/edit"
                                            color: "white"
                                            size: dp(35)
                                            z:1
                                            visible: main.owngxs === mgxs
                                            opacity: mouseArea.containsMouse ? 1 : (mouseArea2.containsMouse ? 0.5 : 0)

                                            Behavior on opacity{
                                                NumberAnimation{duration: MaterialAnimation.pageTransitionDuration/2}
                                            }

                                            MouseArea{
                                                id: mouseArea2
                                                anchors.fill: parent
                                                hoverEnabled: true
                                                onClicked: myProfile.setWallBg()
                                            }
                                        }

                                        Image{
                                            id: bg2
                                            anchors.fill: parent
                                            source: "image://avatar/"+"wallbg/"+mgxs
                                            fillMode: Image.PreserveAspectCrop
                                            visible: false
                                        }

                                        OpacityMask {
                                            id: mask

                                            anchors.fill: parent
                                            maskSource: circleMask
                                            source: bg2
                                        }

                                        Rectangle {
                                            id: circleMask
                                            anchors.fill: parent

                                            smooth: true
                                            visible: false

                                            radius: 10

                                            Rectangle{
                                                anchors.bottom: parent.bottom
                                                anchors.left: parent.left
                                                anchors.right: parent.right
                                                height: dp(30)
                                            }
                                        }
                                    }



                                    Rectangle{
                                        id: avatarRect
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        width: parent.width < parent.height ? parent.width*0.55 : parent.height*0.55
                                        height: parent.width < parent.height ? parent.width*0.55 : parent.height*0.55
                                        radius: width/2

                                        Canvas {
                                            id: canvas
                                            anchors.fill: parent
                                            anchors.margins: dp(2)

                                            Component.onCompleted:loadImage("image://avatar/"+"gxs/"+mgxs);

                                            onPaint: {
                                                var ctx = getContext("2d");
                                                if (canvas.isImageLoaded("image://avatar/"+"gxs/"+mgxs)) {
                                                    var profile = Qt.createQmlObject('import QtQuick 2.5; Image{source: "image://avatar/"+"gxs/"+mgxs;  visible:false}', canvas);
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

                                                onClicked: overlayView.open(canvas)
                                            }
                                        }
                                    }

                                    DropShadow {
                                        anchors.fill: avatarRect
                                        verticalOffset: 3
                                        radius: 5
                                        samples: 7
                                        color: "#80000000"
                                        source: avatarRect
                                    }

                                    Item{
                                        anchors.top: avatarRect.bottom
                                        anchors.topMargin: parent.height*0.05
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        width: parent.width*0.9
                                        height: nameText.implicitHeight

                                        Text{
                                            id: nameText
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            text: name
                                            color: Theme.light.textColor
                                            font.family: "Roboto"
                                            font.pixelSize: dp(18)
                                        }
                                    }
                                }

                                Item{
                                    height: dp(20)
                                    width: parent.width
                                }

                                View{
                                    height: commentsHeader.height
                                    anchors.bottom: parent.bottom
                                    anchors.right: parent.right
                                    anchors.left: parent.left
                                    anchors.bottomMargin: 20
                                    elevation: 1
                                    radius: 10

                                    visible: main.owngxs === mgxs
                                    enabled: main.owngxs === mgxs

                                    Item{
                                        id: commentsHeader
                                        anchors.left: parent.left
                                        anchors.right: parent.right
                                        anchors.top: parent.top
                                        anchors.leftMargin: dp(25)
                                        anchors.rightMargin: dp(25)
                                        height: ((msgBox.lineCount*dp(18))+dp(8)+dp(12)) > dp(50) ? ((msgBox.lineCount*dp(18))+dp(8)+dp(12)+dp(8)) : dp(50)

                                        Canvas {
                                            id: canvas2
                                            anchors.top: parent.top
                                            anchors.left: parent.left
                                            anchors.margins: dp(5)
                                            anchors.leftMargin: dp(0)
                                            anchors.rightMargin: dp(0)
                                            width: dp(40)
                                            height: dp(40)

                                            Component.onCompleted:loadImage("image://avatar/"+"gxs/"+main.owngxs);

                                            onPaint: {
                                                var ctx = getContext("2d");
                                                if (canvas2.isImageLoaded("image://avatar/"+"gxs/"+main.owngxs)) {
                                                    var profile = Qt.createQmlObject('import QtQuick 2.5; Image{source: "image://avatar/"+"gxs/"+main.owngxs;  visible:false}', canvas2);
                                                    var centreX = width/2;
                                                    var centreY = height/2;

                                                    ctx.save();
                                                        ctx.beginPath();
                                                            ctx.arc(centreX, centreY, width / 2, 0, Math.PI*2, true);
                                                        ctx.closePath();
                                                        ctx.clip();
                                                        ctx.drawImage(profile, 0, 0, canvas2.width, canvas2.height);
                                                    ctx.restore();

                                                }
                                            }
                                            onImageLoaded:requestPaint()
                                        }

                                        /*TextArea {
                                            id: msgBox
                                            placeholderText: "Say something..."
                                            height: (lineCount*dp(18))+dp(8)
                                            anchors.top: parent.top
                                            anchors.topMargin: dp(12)
                                            anchors.leftMargin: dp(12)
                                            anchors.left: canvas2.right
                                            anchors.right: parent.right
                                            //anchors.bottom: parent.bottom
                                            textFormat: Text.PlainText

                                            font {
                                                pixelSize: 15 * Units.dp
                                            }

                                            Behavior on height {
                                                ScriptAction { script: {timeView.positionViewAtBeginning()} }
                                            }
                                        }*/

                                        TextField {
                                            id: msgBox
                                            placeholderText: "Say something..."
                                            height: dp(25)
                                            anchors.top: parent.top
                                            anchors.topMargin: dp(12)
                                            anchors.leftMargin: dp(12)
                                            anchors.left: canvas2.right
                                            anchors.right: parent.right
                                            placeholderPixelSize: dp(15)
                                            font {
                                                pixelSize: 15 * Units.dp
                                            }

                                            showBorder: false

                                            onAccepted: {
                                                wallPostModel.createPost(msgBox.text)
                                                msgBox.text = "";
                                            }
                                        }
                                    }
                                }
                            }

                    footer: Item{
                        height: dp(70)
                        width: parent.width

                        IconButton{
                            id: icon_load
                            anchors.centerIn: parent
                            width: dp(40)
                            height: dp(40)
                            iconName: "awesome/chevron_down"
                            color: Theme.light.iconColor
                            size: 35 * Units.dp

                            onEntered: icon_load.color = Theme.primaryColor;
                            onExited: icon_load.color = Theme.light.iconColor;

                            onClicked: {
                                wallPostModel.updateDisplay();
                                timeView.positionViewAtEnd()
                            }
                        }
                    }

                    delegate: PostView{}
                }
            }
        }

        OverlayView {
            id: overlayView

            width: main.width < main.height ? (dp(700)+main.width*0.3 < main.width ? dp(700) : main.width*0.7) : (dp(700)+main.height*0.3 < main.height ? dp(700) : main.height*0.7)
            height: main.width < main.height ? (dp(700)+main.width*0.3 < main.width ? dp(700) : main.width*0.7) : (dp(700)+main.height*0.3 < main.height ? dp(700) : main.height*0.7)

            radiusOnStart: overlayView.width/2

            onOpened: {
                contentImage.radius = 0
            }

            onClosed: {
                contentImage.radius = 150
            }

            Circle.CircleImage{
                id: contentImage
                anchors.fill: parent
                source: Qt.resolvedUrl("image://avatar/"+"gxs/"+mgxs)
                fillMode: Image.PreserveAspectCrop

                Behavior on radius{
                    NumberAnimation {
                        duration: 300; easing.type: Easing.InOutQuad
                    }
                }

                IconButton {
                    anchors {
                        top: parent.top
                        right: parent.right
                        rightMargin: dp(16)
                    }
                    height: dp(60)
                    opacity: overlayView.transitionOpacity

                    visible: main.owngxs === mgxs
                    iconName: "awesome/edit"

                    color: Theme.dark.iconColor
                    size: dp(40)

                    onClicked: myProfile.setAvatar(mgxs)
                }
            }
        }

        ParallelAnimation {
            running: true
            NumberAnimation { target: content; property: "anchors.bottomMargin"; from: -50; to: 0; duration: MaterialAnimation.pageTransitionDuration }
            NumberAnimation { target: content; property: "opacity"; from: 0; to: 1; duration: MaterialAnimation.pageTransitionDuration }
        }
    }
}
