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
import Material.Extras 0.1

Dialog {
    positiveButtonText: "Cancel"
    negativeButtonText: "Add"

    positiveButtonSize: 13
    negativeButtonSize: 13

    onRejected: {
        friendList.addFriend(friendCert.text)
    }


    Label {
        id: titleLabel

        anchors.left: parent.left
        verticalAlignment: Text.AlignVCenter
        height: dp(50)
        wrapMode: Text.Wrap
        text: "Add Friend"
        style: "title"
        color: Theme.accentColor
    }

    Grid {
        width: main.width < dp(800) ? main.width - dp(100) : dp(700)
        height: main.width < dp(300) ? main.width - dp(100) : dp(300)
        spacing: dp(8)

        Item{

            width: parent.width/2
            height: parent.height

            Text {
                height: 35
                text: "It's your certificate. Share it with friends."
                anchors.left: parent.left
                anchors.right: parent.right
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                color: Theme.light.textColor
                font {
                    family: "Roboto"
                    pixelSize: 16 * Units.dp
                }
            }

            TextArea {
                text: myProfile.showMyCert().replace(/(\r\n|\n|\r)/gm,"")
                anchors.fill: parent
                anchors.topMargin: 35
                textFormat: Text.PlainText
                wrapMode: Text.WrapAnywhere
                //readOnly: true
                font {
                    pixelSize: 12 * Units.dp
                }
            }
        }

        Item{
            width: parent.width/2
            height: parent.height

            Text {
                height: 35
                text: "Paste your friend's certificate here"
                anchors.left: parent.left
                anchors.right: parent.right
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                font {
                    family: "Roboto"
                    pixelSize: 16 * Units.dp
                }
            }

            TextArea {
                id: friendCert
                placeholderText: myProfile.showMyCert().replace(/(\r\n|\n|\r)/gm,"")
                anchors.fill: parent
                anchors.topMargin: 35
                textFormat: Text.PlainText
                wrapMode: Text.WrapAnywhere
                font {
                    pixelSize: 12 * Units.dp
                }
            }
        }
    }
}
