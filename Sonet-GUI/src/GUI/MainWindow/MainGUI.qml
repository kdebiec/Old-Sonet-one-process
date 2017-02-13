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
import Material.Extras 0.1

import QtQuick.Controls 1.3 as Controls

Rectangle
{
    id: main
    color: Palette.colors["grey"]["200"]

    property bool borderless: false

    property Item controls: controlView
    property string owngxs: myProfile.getPreferredGxs()

    signal gridChanged

    property alias pageStack: __pageStack
    property alias gridLayout: gridLayout
    property alias content: content

    property int visibleRows: Math.round((main.height-dp(30))/(50 + gridLayout.rowSpacing))

    Connections{
        target: view
        onHeightChanged: gridLayout.reorder()
        onWidthChanged: gridLayout.reorder()
    }

    Connections{
        target: voipHandler
        onVoipConnection: {
            var component = Qt.createComponent("CallCard.qml");
            if (component.status === Component.Ready) {
                var card = component.createObject(gridLayout, {"rsPeerId": rsPeerId});
                card.col = 5;
                card.row = 5;
                card.gridX = gridLayout.columns - card.col;
                card.name = name;
                gridLayout.reorder()
            }
        }
    }

    Component.onCompleted: {updateVisibleRows(); haveOwnId()}

    function haveOwnId()
    {
        if (!myProfile.haveIdentity()) {
            main.content.activated = false;
            var component = Qt.createComponent("CreateIdentity.qml");
            if (component.status === Component.Ready) {
                var createId = component.createObject(main);
                createId.show();
            }
        }
    }

    /*
     * Global GUI Settings
     */
    property bool flickableGridMode: settings.getFlickableGridMode()
    property bool advmode: settings.getAdvancedMode()
    // ~Settings

    AppTheme {
        id: __theme

        primaryColor: Palette.colors["green"]["500"]
        accentColor: Palette.colors["deepOrange"]["500"]
        tabHighlightColor: "white"
    }

    Image{
        anchors.left: leftBar.right
        anchors.right: rightBar.left
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        source: "/wallpaper_grey.jpg"
        fillMode: Image.PreserveAspectCrop
    }

    View {
        id: logoView
        backgroundColor: "white"
        elevation: 2
        anchors.top: parent.top
        anchors.left: parent.left
        z: 1

        height: dp(50)
        width: dp(50)

        ParallelAnimation {
            running: true
            NumberAnimation { target: logoView; property: "anchors.leftMargin"; from: -50; to: 0; duration: MaterialAnimation.pageTransitionDuration }
            NumberAnimation { target: logoView; property: "opacity"; from: 0; to: 1; duration: MaterialAnimation.pageTransitionDuration }
        }
    }

    LeftBar{
        id: leftBar
        z:1
    }

    View {
        id: controlView
        backgroundColor: "white"
        elevation: 2
        anchors.top: parent.top
        anchors.right: parent.right
        z: 1

        height: dp(50)
        width: dp(210)

        ParallelAnimation {
            running: true
            NumberAnimation { target: controlView; property: "anchors.rightMargin"; from: -50; to: 0; duration: MaterialAnimation.pageTransitionDuration }
            NumberAnimation { target: controlView; property: "opacity"; from: 0; to: 1; duration: MaterialAnimation.pageTransitionDuration }
        }

        Component.onCompleted: {
            if(borderless)
                Qt.createQmlObject('
                import QtQuick 2.5
                import Material 0.3
                import QtQuick.Layouts 1.3

                Row {
                    anchors {
                        top: parent.top
                        right: parent.right
                        rightMargin: dp(10)
                        topMargin: dp(12)
                    }

                    spacing: 5 * Units.dp

                    Item{
                        width: dp(26)
                        height: dp(26)
                        Rectangle{
                            id: minimizeButton
                            anchors.bottom: parent.bottom
                            width: parent.width-8
                            height: dp(2)
                            anchors.margins: dp(4)
                            color: Palette.colors["grey"]["500"]
                        }

                        MouseArea{
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: minimizeButton.color = Theme.accentColor
                            onExited: minimizeButton.color = Palette.colors["grey"]["500"]
                            onClicked: qMainPanel.pushButtonMinimizeClicked();
                        }
                    }

                    Item{
                        width: dp(26)
                        height: dp(26)

                        Rectangle{
                            id: maximizeButton
                            anchors.fill: parent
                            anchors.margins: dp(4)
                            color: Palette.colors["grey"]["500"]

                            Rectangle{
                                anchors.fill: parent
                                anchors.margins: dp(2)
                                color: "white"
                            }


                        }
                        MouseArea{
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: maximizeButton.color = Theme.accentColor
                            onExited: maximizeButton.color = Palette.colors["grey"]["500"]
                            onClicked: qMainPanel.pushButtonMaximizeClicked();
                        }
                    }

                    Item{
                        width: dp(26)
                        height: dp(26)

                        Rectangle{
                            id: closeButton
                            width: dp(24)
                            height: dp(2.5)
                            anchors.centerIn: parent
                            rotation: 45
                            color: Palette.colors["grey"]["500"]
                        }

                        Rectangle{
                            id: closeButton2
                            width: dp(24)
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
                            onClicked: view.hide()
                        }
                    }
                }
                ', controlView);
        }
    }

    RightBar{
        id: rightBar
        z:1
    }

    Flickable {
        id: flickable
        clip: true
        interactive: flickableGridMode
        contentHeight: Math.max(gridLayout.implicitHeight + 40, height)
        anchors {
            left: leftBar.right
            right: rightBar.left
            top: parent.top
            bottom: parent.bottom
        }

        GridLayout {
            id: gridLayout

            property int h: (main.height-dp(30))
            property int rowspace: parseInt(h/dp(50))
            anchors.fill: parent
            anchors.margins: dp(15)
            columnSpacing: dp(10)
            rowSpacing: h<650 ? (h-((rowspace-1)*50))/(rowspace-2) : (h-((rowspace-2)*50))/(rowspace-3)
            columns: parseInt(gridLayout.width / dp(60))

            property alias gridRepeater: gridRepeater

            onColumnsChanged: main.gridChanged()

            function reorder() {
                for (var i=0;i<(children.length - 751);i++) {
                  children[751 + i].refresh()
                }
            }

            Repeater {
                id: gridRepeater
                model: 750
                delegate: DropTile {}

                Layout.alignment: Qt.AlignTop
                signal activeGrid
                signal nonActiveGrid
            }

            DragTile{
                id: content

                Layout.alignment: Qt.AlignBottom
                Layout.maximumWidth: 0
                Layout.maximumHeight: 0
                width: 0
                height: 0
                col: parseInt(gridLayout.width / (50 + gridLayout.columnSpacing))>= 9 ? 9 : parseInt(gridLayout.width / (50 + gridLayout.columnSpacing))
                row: main.visibleRows

                gridX: Math.floor(((parseInt(gridLayout.width / (50 + gridLayout.columnSpacing)))-content.col)/2)

                Behavior on col {
                    ScriptAction { script: {content.refresh();gridLayout.reorder();} }
                }

                Behavior on row {
                    ScriptAction { script: {content.refresh()} }
                }

                Controls.StackView{
                    id: __pageStack
                    initialItem:    Content{
                                        mgxs: myProfile.getPreferredGxs()
                                        name: myProfile.getNickBaseIdentity()
                                    }
                    anchors.fill: parent
                }
            }
        }
    }

    Scrollbar {
        flickableItem: flickable
    }

    OverlayLayer {
        id: dialogOverlayLayer
        objectName: "dialogOverlayLayer"
        z: 10
    }

    OverlayLayer {
        id: tooltipOverlayLayer
        objectName: "tooltipOverlayLayer"
        z:5
    }

    OverlayLayer {
        id: overlayLayer
    }

    // Dialog Pop-ups

    DialogExit {
        id: colorPicker
        text: "Do you want to exit?"

        positiveButtonText: "Yes"
        negativeButtonText: "No"

        onAccepted: Qt.quit()
    }

    SettingsDialog{
        id: settingsDialog
    }

    UserAddDialog{
        id: userAddDialog
    }

    function updateVisibleRows()
    {
        main.visibleRows = Qt.binding(function() { return Math.round((main.height-dp(30))/(50 + gridLayout.rowSpacing))});
    }

    /*
      All new cards (panels) are instatiated in these fucntions,
      becouse creation context is the QQmlContext in which Qt.createComponent method is called.
      If it would be created e.g. in FriendListDelegate we could lost access to new created objects.
      (We couldn't e.g. click on mousearea in new created objects)
      */
    function createChatCard(friendname, rspgpid, objectName, gxsid)
    {
        var component = Qt.createComponent(objectName, gridLayout);
        if (component.status === Component.Ready) {
            var chat = component.createObject(gridLayout);
            chat.gxsId = gxsid
            chat.name = friendname
            chat.rspgpid = rspgpid
            chat.col = 5;
            updateVisibleRows()
            chat.row = main.visibleRows;
            chat.gridX = gridLayout.columns - chat.col;
            //chat.gridY = 5;
            gridLayout.reorder()
        }
    }

    function createChatCardPGP(friendname, rspgpid, objectName)
    {
        var component = Qt.createComponent(objectName, gridLayout);
        if (component.status === Component.Ready) {
            var chat = component.createObject(gridLayout);
            chat.name = friendname
            chat.rspgpId = rspgpid
            chat.col = 5;
            updateVisibleRows()
            chat.row = main.visibleRows;
            chat.gridX = gridLayout.columns - chat.col;
            //chat.gridY = 5;
            gridLayout.reorder()
        }
    }

    function createCallCard(gxsid, friendname, objectName)
    {
        var component = Qt.createComponent(objectName, gridLayout);
        if (component.status === Component.Ready) {
            var card = component.createObject(gridLayout, {"gxsId": gxsid});
            card.name = friendname
            card.col = 5;
            card.row = 5;
            card.gridX = gridLayout.columns - card.col;
            gridLayout.reorder()
        }
    }

    function createCallCardPGP(rspgpid, friendname, objectName)
    {
        var component = Qt.createComponent(objectName, gridLayout);
        if (component.status === Component.Ready) {
            var card = component.createObject(gridLayout, {"rspgpId": rspgpid});
            card.name = friendname
            card.col = 5;
            card.row = 5;
            card.gridX = gridLayout.columns - card.col;
            gridLayout.reorder()
        }
    }

    // Units
    function dp(dp) {
        return dp * Units.dp
    }

    function gu(gu) {
        return units.gu(gu)
    }

    UnitsHelper {
        id: units
    }
}
