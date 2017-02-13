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
import QtGraphicalEffects 1.0

import Material 0.3
import Material.Extras 0.1 as Circle
import Material.ListItems 0.1 as ListItem

import Sonet 1.0

DragTile{
    id: drag

    property string rspgpId
    property string gxsId// only for avatar
    property string name

    property bool muted: false
    property bool mutedReceiver: false

    // Just for "restore" option
    property int tmpCol
    property int tmpRow
    property int tmpGridX: 0   // Numbering starts from 0
    property int tmpGridY: 0   // Numbering starts from 0
    property bool maximized: false
    //

    Layout.alignment: Qt.AlignBottom
    Layout.maximumWidth: 0
    Layout.maximumHeight: 0
    width: 0
    height: 0
    col: 5
    row: 3

    ParallelAnimation {
        running: true
        NumberAnimation { target: drag; property: "opacity"; from: 0; to: 1; duration: MaterialAnimation.pageTransitionDuration/2 }
    }

    VOIP{
        id: voip
        Component.onCompleted: voip.rspgpid = drag.rspgpId//drag.rspgpId === "" ? voip.rspeerid = drag.rsPeerId : voip.rspgpid = drag.rspgpId
        onAcceptReceived:{
            progressCircle.visible = false;
            canvas.width = Qt.binding(function() { return (cardItem.width*0.53 + dp(40)) < ( row.y - cardItem.height*0.05) ? cardItem.width*0.53 : (row.y - cardItem.height*0.05 - dp(40))})
            canvas.height = Qt.binding(function() { return (cardItem.width*0.53 + dp(40)) < ( row.y - cardItem.height*0.05) ? cardItem.width*0.53 : (row.y - cardItem.height*0.05 - dp(40))})
        }
        onAcceptSended:{
            progressCircle.visible = false;
            canvas.width = Qt.binding(function() { return (cardItem.width*0.53 + dp(40)) < ( row.y - cardItem.height*0.05) ? cardItem.width*0.53 : (row.y - cardItem.height*0.05 - dp(40))})
            canvas.height = Qt.binding(function() { return (cardItem.width*0.53 + dp(40)) < ( row.y - cardItem.height*0.05) ? cardItem.width*0.53 : (row.y - cardItem.height*0.05 - dp(40))})
        }
        onHangupReceived: drag.destroy()
    }

    MouseArea{
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: overflowMenu7.open(drag, mouse.x, mouse.y);

        Dropdown {
            id: overflowMenu7
            objectName: "overflowMenu7"
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
                        overflowMenu7.close()
                        if(!maximized)
                        {
                            drag.tmpGridX = drag.gridX
                            drag.tmpGridY = drag.gridY
                            drag.tmpCol = drag.col
                            drag.tmpRow = drag.row
                            drag.gridX = 0
                            drag.gridY = 0
                            drag.col = Qt.binding(function() { return parseInt(gridLayout.width / dp(60)) || 1})
                            drag.row = Qt.binding(function() { return main.visibleRows})//parseInt(main.height / dp(60)) || 1})
                            maximized = true
                        }
                        else if(maximized)
                        {
                            drag.gridX = drag.tmpGridX
                            drag.gridY = drag.tmpGridY
                            drag.col = drag.tmpCol
                            drag.row = drag.tmpRow
                            maximized = false
                        }

                        drag.refresh()

                    }
                }

                ListItem.Standard {
                    height: dp(30)
                    text: "Close"
                    itemLabel.style: "menu"
                    onClicked: {
                        overflowMenu7.close()
                        drag.destroy()
                    }
                }
            }
        }
    }

    View {
        id: card
        anchors.fill: parent
        elevation: 2
        backgroundColor: Theme.tabHighlightColor


        Behavior on anchors.topMargin {
            NumberAnimation { duration: MaterialAnimation.pageTransitionDuration }
        }

        Rectangle{
            anchors.fill: parent
            z: 1
            color: Qt.rgba(0,0,0, 0.1)
        }

        Item{
            id: cardItem
            anchors.fill: parent
            z: 1

            DropShadow {
                anchors.fill: canvas
                verticalOffset: 2
                radius: 12
                samples: 12
                color: "#50000000"
                source: canvas
            }

            Canvas {
                id: canvas
                y: parent.height*0.05
                anchors.horizontalCenter: parent.horizontalCenter
                width: (parent.width*0.45 + dp(70)) < ( row.y - parent.height*0.05) ? parent.width*0.45 : (row.y - parent.height*0.05 - dp(70))
                height: (parent.width*0.45 + dp(70)) < ( row.y - parent.height*0.05) ? parent.width*0.45 : (row.y - parent.height*0.05 - dp(70))

                Component.onCompleted:loadImage("image://avatar/"+"gxs/"+gxsId);

                onPaint: {
                    var ctx = getContext("2d");
                    if (canvas.isImageLoaded("image://avatar/"+"gxs/"+gxsId)) {
                        var profile = Qt.createQmlObject('import QtQuick 2.5; Image{source: "image://avatar/"+"gxs/"+gxsId;  visible:false}', canvas);
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
                    acceptedButtons: Qt.RightButton | Qt.LeftButton

                    Rectangle{
                        anchors.fill: parent
                        color: "black"
                        opacity: circleInk.containsMouse ? 0.1 : 0
                        radius: width/2
                    }

                    onClicked: {
                        if(mouse.button === Qt.LeftButton)
                            overlayView.open(canvas)
                        else if(mouse.button === Qt.RightButton)
                            overflowMenu11.open(circleInk, mouse.x, mouse.y);
                    }

                    Dropdown {
                        id: overflowMenu11
                        objectName: "overflowMenu11"
                        overlayLayer: "dialogOverlayLayer"
                        width: 200 * Units.dp
                        height: dp(3*30)
                        enabled: true
                        anchor: Item.TopLeft
                        durationSlow: 200
                        durationFast: 100

                        Column{
                            anchors.fill: parent

                            ListItem.Standard {
                                height: dp(30)
                                text: mutedReceiver === false ? "Mute" : "Unmute"
                                itemLabel.style: "menu"
                                onClicked: {
                                    overflowMenu11.close()
                                    mutedReceiver = !mutedReceiver
                                    if(mutedReceiver)
                                        voip.muteReceiver()
                                    else
                                        voip.unmuteReceiver()
                                }
                            }
                            Rectangle{
                                id: line
                                height: dp(1)
                                width: parent.width
                                color: Palette.colors["grey"]["200"]
                            }
                            Item{
                                height: dp(60)
                                width: parent.width
                                Item{
                                    height: dp(30)
                                    width: parent.width
                                    Label {
                                        id: label

                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.left: parent.left
                                        anchors.leftMargin: dp(17)

                                        text: "Volume"
                                        style: "menu"

                                        color: Theme.light.textColor
                                    }
                                }
                                Slider {
                                    id: slider
                                    anchors.centerIn: parent
                                    width: dp(174)
                                    height: dp(30)
                                    value: 100

                                    numericValueLabel: true
                                    stepSize: 1
                                    minimumValue: 0
                                    maximumValue: 100
                                    knobLabel: value + "%"
                                    knobDiameter: dp(35)

                                    Behavior on knobLabel{
                                        ScriptAction {
                                            script: {
                                                voip.setOutputVolume(slider.value)
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Text{
                id: nameText
                anchors.top: canvas.bottom
                anchors.topMargin: dp(5)
                anchors.horizontalCenter: parent.horizontalCenter
                text: name
                color: Theme.dark.textColor
                font.family: "Roboto"
                font.pixelSize: 22
            }

            ProgressCircle {
                id: progressCircle
                anchors.top: nameText.bottom
                anchors.topMargin: dp(2)
                anchors.horizontalCenter: parent.horizontalCenter
                color: Theme.accentColor
                width: dp(32)
                height: dp(32)
                dashThickness: dp(5)
            }

            Row{
                id: row
                anchors.bottom: parent.bottom
                anchors.bottomMargin: parent.height*0.06
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 10

                View{
                    anchors.verticalCenter: parent.verticalCenter
                    height: (drag.height > dp(500) & drag.width > 500) ? dp(45) : dp(40)
                    width: (drag.height > dp(500) & drag.width > 500) ? dp(45) : dp(40)
                    elevation: 3
                    radius: width/2
                    backgroundColor: Theme.primaryColor

                    IconButton {
                        anchors.fill: parent
                        size: 20
                        color: "white"
                        iconName: "awesome/video_camera"
                    }
                }

                View{
                    height: (drag.height > dp(500) & drag.width > 500) ? dp(55) : dp(50)
                    width: (drag.height > dp(500) & drag.width > 500) ? dp(55) : dp(50)
                    elevation: 3
                    radius: width/2
                    backgroundColor: Theme.primaryColor
                    IconButton {
                        anchors.fill: parent
                        size: 30
                        color: "white"
                        iconName: muted === false ? "awesome/microphone" : "awesome/microphone_slash"

                        onClicked: {
                            muted = !muted
                            if(muted)
                                voip.muteMic()
                            else
                                voip.unmuteMic()
                        }
                    }
                }

                View{
                    anchors.verticalCenter: parent.verticalCenter
                    height: (drag.height > dp(500) & drag.width > 500) ? dp(45) : dp(40)
                    width: (drag.height > dp(500) & drag.width > 500) ? dp(45) : dp(40)
                    elevation: 3
                    radius: width/2
                    backgroundColor: Theme.accentColor

                    IconButton {
                        anchors.fill: parent
                        size: 20
                        color: "white"
                        iconName: "awesome/phone"
                        rotation: 135

                        onClicked: drag.destroy()
                    }
                }
            }
        }

        Image{
            id: bg
            anchors.fill: parent
            source: "image://avatar/"+"wallbg/"+gxsId
            fillMode: Image.PreserveAspectCrop
        }
        FastBlur {
            anchors.fill: bg
            source: bg
            radius: 32
        }

        OverlayView {
            id: overlayView

            width: main.width < main.height ? (dp(700)+main.width*0.3 < main.width ? dp(700) : main.width*0.7) : (dp(700)+main.height*0.3 < main.height ? dp(700) : main.height*0.7)
            height: main.width < main.height ? (dp(700)+main.width*0.3 < main.width ? dp(700) : main.width*0.7) : (dp(700)+main.height*0.3 < main.height ? dp(700) : main.height*0.7)

            radiusOnStart: overlayView.width/2

            onOpened: {
                contentImage.radius = 0
                rowOverlay.visible = true
            }

            onClosed: {
                rowOverlay.visible = false
                contentImage.radius = 150
            }

            Circle.CircleImage{
                id: contentImage
                anchors.fill: parent
                source: Qt.resolvedUrl("image://avatar/"+"gxs/"+gxsId)
                fillMode: Image.PreserveAspectCrop

                Behavior on radius{
                    NumberAnimation {
                        duration: 300; easing.type: Easing.InOutQuad
                    }
                }

                Row{
                    id: rowOverlay
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: parent.height*0.06
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 10

                    View{
                        anchors.verticalCenter: parent.verticalCenter
                        height: (drag.height > dp(500) & drag.width > 500) ? dp(45) : dp(40)
                        width: (drag.height > dp(500) & drag.width > 500) ? dp(45) : dp(40)
                        elevation: 6
                        radius: width/2
                        backgroundColor: Theme.primaryColor

                        IconButton {
                            anchors.fill: parent
                            size: 20
                            color: "white"
                            iconName: "awesome/video_camera"
                        }
                    }

                    View{
                        height: (drag.height > dp(500) & drag.width > 500) ? dp(55) : dp(50)
                        width: (drag.height > dp(500) & drag.width > 500) ? dp(55) : dp(50)
                        elevation: 6
                        radius: width/2
                        backgroundColor: Theme.primaryColor
                        IconButton {
                            anchors.fill: parent
                            size: 30
                            color: "white"
                            iconName: muted === false ? "awesome/microphone" : "awesome/microphone_slash"

                            onClicked: {
                                muted = !muted
                                if(muted)
                                    voip.muteMic()
                                else
                                    voip.unmuteMic()
                            }
                        }
                    }

                    View{
                        anchors.verticalCenter: parent.verticalCenter
                        height: (drag.height > dp(500) & drag.width > 500) ? dp(45) : dp(40)
                        width: (drag.height > dp(500) & drag.width > 500) ? dp(45) : dp(40)
                        elevation: 6
                        radius: width/2
                        backgroundColor: Theme.accentColor

                        IconButton {
                            anchors.fill: parent
                            size: 20
                            color: "white"
                            iconName: "awesome/phone"
                            rotation: 135

                            onClicked: drag.destroy()
                        }
                    }
                }
            }
        }
    }
}
