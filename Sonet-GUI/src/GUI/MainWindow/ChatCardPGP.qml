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
import Material.ListItems 0.1 as ListItem

import Sonet 1.0

DragTile{
    id: drag

    property string rspgpId
    property string name
    property alias contentm: contentm

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
    opacity: 0

    Behavior on gridX {
        ScriptAction { script: {drag.refresh()} }
    }

    Behavior on row {
        ScriptAction { script: {contentm.positionViewAtEnd()} }
    }

    ParallelAnimation {
        running: true
        SequentialAnimation{
            NumberAnimation {duration: 50 }
            NumberAnimation { target: drag; property: "opacity"; from: 0; to: 1; duration: MaterialAnimation.pageTransitionDuration/2 }
        }
    }

    StatusColor{
        id: statusColor
        rspgp: rspgpId
    }

    View {
        id: chat

        anchors.fill: parent
        elevation: 2
        backgroundColor: Palette.colors["grey"]["50"]

        Behavior on anchors.topMargin {
            NumberAnimation { duration: MaterialAnimation.pageTransitionDuration }
        }

        Rectangle{
            id: chatHeader
            height: 35
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            color: Palette.colors["grey"]["50"]
            z: 2

            MouseArea{
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: overflowMenu5.open(drag, mouse.x, mouse.y);

                Item{
                    anchors.bottom: parent.bottom
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width > (9*60+dp(30)) ? 9*60 : (parent.width-dp(30))

                    Rectangle{
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        height: dp(1)
                        color: Palette.colors["grey"]["200"]
                    }

                    Text{
                        id: headertext
                        text: name
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: dp(20)
                        anchors.left: parent.left
                        font.pixelSize: dp(17)
                        font.family: "Roboto"
                        color: statusColor.color
                    }

                    Item{
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: dp(18)
                        width: dp(23)
                        height: dp(23)

                        Rectangle{
                            id: closeButton
                            width: dp(20)
                            height: dp(2.5)
                            anchors.centerIn: parent
                            rotation: 45
                            color: Palette.colors["grey"]["500"]
                        }

                        Rectangle{
                            id: closeButton2
                            width: dp(20)
                            height: dp(2.5)
                            anchors.centerIn: parent
                            rotation: -45
                            color: Palette.colors["grey"]["500"]
                        }

                        MouseArea{
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: {closeButton.color = Theme.accentColor; closeButton2.color = Theme.accentColor}
                            onExited: {closeButton.color = Palette.colors["grey"]["500"]; closeButton2.color = Palette.colors["grey"]["500"]}
                            onClicked: drag.destroy()
                        }
                    }
                }

                Dropdown {
                    id: overflowMenu5
                    objectName: "overflowMenu5"
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
                                overflowMenu5.close()
                                if(!maximized)
                                {
                                    drag.tmpGridX = drag.gridX
                                    drag.tmpGridY = drag.gridY
                                    drag.tmpCol = drag.col
                                    drag.tmpRow = drag.row
                                    drag.gridX = 0
                                    drag.gridY = 0
                                    drag.col = Qt.binding(function() { return parseInt(gridLayout.width / dp(60))>= 11 ? 11 : parseInt(gridLayout.width / dp(60)) || 1})
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
                                overflowMenu5.close()
                                drag.destroy()
                            }
                        }
                    }
                }
            }
        }

        DropShadow {
            anchors.fill: chatHeader
            verticalOffset: 5
            radius: 30
            samples: 61
            color: Palette.colors["grey"]["50"]
            source: chatHeader
            z: 1
        }

        Item{
            anchors.top: chatHeader.bottom
            anchors.bottom: chatFooter.top
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width > (9*60+dp(30)) ? 9*60 : (parent.width-dp(30))

            Item{
                anchors.fill: parent
                anchors.margins: 2

                ListView{
                    id: contentm
                    anchors.fill: parent
                    anchors.leftMargin: 5
                    anchors.rightMargin: 5

                    clip: true
                    snapMode: ListView.NoSnap
                    flickableDirection: Flickable.AutoFlickDirection

                    model:  ChatMsgModel{
                                id: msgModel;
                                rspgp: rspgpId

                                onUpdatedDisplay: contentm.positionViewAtEnd()
                            }

                    delegate: ChatMsgDelegate{}
                }

                Scrollbar {
                    flickableItem: contentm
                    anchors.margins: 0
                }
            }
        }

        Item{
            id: chatFooter
            height: (msgBox.contentHeight < dp(20) ? (msgBox.contentHeight+30) : (msgBox.contentHeight+22)) < dp(200) ? (msgBox.contentHeight < dp(20) ? (msgBox.contentHeight+30) : (msgBox.contentHeight+22)) : dp(200)
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            z: 1

            Behavior on height {
                ScriptAction{script: contentm.positionViewAtEnd()}
            }

            View{
                id: footerView
                anchors.bottom: parent.bottom
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: dp(10)
                width: parent.width > (9*60+dp(30)) ? 9*60 : (parent.width-dp(30))
                radius: 10
                elevation: 1
                backgroundColor: "white"

                TextArea{
                    id: msgBox
                    placeholderText: footerView.width > dp(195) ? "Say hello to your friend" : "Say hello"
                    anchors.fill: parent
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.topMargin: dp(5)
                    anchors.bottomMargin: dp(5)
                    anchors.leftMargin: dp(18)
                    anchors.rightMargin: dp(18)

                    wrapMode: Text.WordWrap
                    frameVisible: false
                    horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                    verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff

                    focus: true
                    font.pixelSize: 15 * Units.dp

                    onActiveFocusChanged: {
                        if(activeFocus)
                        {
                            msgStore.cleanStoredMsgForPGP(rspgpid)
                            footerView.elevation = 2
                        }
                        else
                            footerView.elevation = 1
                    }

                    Keys.onPressed:{
                        if(event.key == Qt.Key_Return)
                        {
                            chatHandler.sendMsgViaPGP(drag.rspgpId, msgBox.text);
                            msgModel.updateDisplay();
                            msgBox.text = "";
                            event.accepted = true;
                        }
                    }
                }
            }
        }
    }
}
