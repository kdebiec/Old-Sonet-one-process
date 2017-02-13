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

Item {
    width: parent.width
    height: textMsg.implicitHeight + dp(26) > dp(50) ? textMsg.implicitHeight + dp(26) : dp(50)

    Canvas {
        id: canvas2
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: dp(5)
        anchors.leftMargin: dp(0)
        anchors.rightMargin: dp(0)
        width: dp(40)
        height: dp(40)

        Component.onCompleted:loadImage("image://avatar/"+"gxs/"+comgxsid);

        onPaint: {
            var ctx = getContext("2d");
            if (canvas2.isImageLoaded("image://avatar/"+"gxs/"+comgxsid)) {
                var profile = Qt.createQmlObject('import QtQuick 2.5; Image{source: "image://avatar/"+"gxs/"+comgxsid;  visible:false}', canvas2);
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

    Text {
        id: nameText
        height: dp(20)
        anchors.top: parent.top
        anchors.left: canvas2.right
        width: implicitWidth

        anchors.leftMargin: dp(15)
        anchors.topMargin: dp(3)

        text: friendname
        color: Palette.colors["green"]["700"]
        font.family: "Roboto"
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
        font.pixelSize: 12
    }

    Text {
        id: dateText
        height: dp(20)
        anchors.top: parent.top
        anchors.left: nameText.right
        width: implicitWidth

        anchors.leftMargin: dp(15)
        anchors.topMargin: dp(3)

        text: date
        color: Palette.colors["grey"]["600"]
        font.family: "Roboto"
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
        font.pixelSize: 11
    }

    TextEdit {
        id: textMsg
        text: contentmsg
        anchors.top: nameText.bottom
        anchors.leftMargin: dp(15)
        anchors.left: canvas2.right
        anchors.right: parent.right
        textFormat: Text.RichText
        wrapMode: Text.WordWrap
        color: Theme.light.textColor
        readOnly: true
        selectByMouse: true
        selectionColor: Theme.accentColor

        font {
            family: "Roboto"
            pixelSize: 14 * Units.dp
        }
    }
}
