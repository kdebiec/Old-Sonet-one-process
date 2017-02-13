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
import Material.ListItems 0.1 as ListItem

View {
    id: leftBar

    property string color: "white"
    property string selectedPage: pageStack.currentItem.title

    anchors {
        top: parent.top
        left: parent.left
        bottom: parent.bottom
        topMargin: dp(50)
    }

    width: dp(50)

    backgroundColor: Theme.tabHighlightColor
    elevation: 2

    clipContent: true

    Column {
        width: parent.width
        height: parent.height*0.7

        ListModel {
            id: sideModel

            ListElement {
                src: "avatar"
                icon: false
                page: "Content.qml"
                pageTitle: "profilePage"
                helperName: "Profile"
            }
        }

        Repeater {
            model: sideModel
            delegate: SideImg{
                name: helperName
                srcIcon: src === "avatar" ?  "image://avatar/"+"gxs/"+main.owngxs : src
                isIcon: icon
                margins: 0
                selected: selectedPage === pageTitle
                onClicked: {
                    main.content.activated = true;

                    if(helperName === "Profile")
                    {
                        wallPostModel.loadWall(owngxs);
                        pageStack.push({item: Qt.resolvedUrl(page), immediate: true, replace: true, properties: {name:  myProfile.getNickBaseIdentity(), mgxs: myProfile.getPreferredGxs()}})
                    }
                    else
                        pageStack.push({item: Qt.resolvedUrl(page), replace: true})
                }
            }
        }
    }

    Column {
        width: parent.width
        anchors.bottom: parent.bottom

        ListModel {
            id: sideModel2

            ListElement {
                src: "awesome/user_plus"
                icon: true
                index: 0
                helperName: "Add Friend"
            }
            ListElement {
                src: "awesome/cog"
                icon: true
                rt: true
                index: 1
                helperName: "Settings"
            }
            ListElement {
                src: "awesome/sign_out"
                icon: true
                index: 2
                helperName: "Exit"
            }
        }

        Repeater {
            model: sideModel2
            delegate: SideImg{
                name: helperName
                srcIcon: src
                isIcon: icon
                margins: 0
                rotate: rt
                onClicked: {
                    switch(index) {
                        case 0:
                            userAddDialog.show()
                            break;
                        case 1:
                            settingsDialog.show()
                            break;
                        case 2:
                            colorPicker.show()
                            break;
                    }
                }
            }
        }
    }

    SequentialAnimation{
        running: true
        ParallelAnimation {
            NumberAnimation { target: leftBar; property: "anchors.leftMargin"; from: -50; to: 0; duration: MaterialAnimation.pageTransitionDuration }
            NumberAnimation { target: leftBar; property: "opacity"; from: 0; to: 1; duration: MaterialAnimation.pageTransitionDuration }
        }
        ScriptAction { script: {control.notifyEnable()} }
    }
}
