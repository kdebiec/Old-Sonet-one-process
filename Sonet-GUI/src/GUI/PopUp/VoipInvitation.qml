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
import QtGraphicalEffects 1.0

import Material 0.3

Item {
    width: 240
    height: 280

    property string gxsId: ""

    MouseArea{
        id: globalMA
        anchors.fill: parent

        onEntered: popup.pressed(globalMA.mouseX, globalMA.mouseY);

        View{
            id: viewLayout
            anchors.fill: parent
            anchors.margins: 15
            anchors.bottomMargin: 25
            elevation: 3

            backgroundColor: Qt.rgba(0,0,0, 0.5)

            Column{
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 13
                Item{
                    height: 5
                    width: 1
                }

                Canvas {
                    id: canvas
                    width: 110
                    height: 110
                    anchors.horizontalCenter: parent.horizontalCenter


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
                }

                Text{
                    id: nameText
                    width: viewLayout.width
                    height: 23
                    text: friendname
                    color: "white"
                    font.family: "Roboto"
                    font.pixelSize: 20
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }

                Row{
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 23

                    View{
                        id: view
                        anchors.verticalCenter: parent.verticalCenter
                        height: 50
                        width: 50
                        elevation: 1
                        radius: width/2
                        backgroundColor: Palette.colors["green"]["500"]

                        IconButton {
                            id: button
                            anchors.fill: parent
                            size: 20
                            color: "white"
                            iconName: "awesome/phone"
                            rotation: 0

                            onClicked: {
                                voip.accept(peerid, friendname)
                                Qt.quit()
                            }
                        }
                    }

                    View{
                        id: view2
                        anchors.verticalCenter: parent.verticalCenter
                        height: 50
                        width: 50
                        elevation: 1
                        radius: width/2
                        backgroundColor: Palette.colors["deepOrange"]["500"]

                        IconButton {
                            id: button2
                            anchors.fill: parent
                            size: 20
                            color: "white"
                            iconName: "awesome/phone"
                            rotation: 135
                            z:1

                            onClicked:
                            {
                                voip.hangup(peerid);
                                Qt.quit()
                            }
                        }
                    }
                }
            }
        }
    }
}
