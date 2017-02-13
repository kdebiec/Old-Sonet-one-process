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
import QtQuick.Controls 1.4 as QtControls

import Material 0.3
import Material.Extras 0.1
import Material.ListItems 0.1 as ListItem

Dialog {
    id: scrollingDialog

    positiveButtonText: "Cancel"
    negativeButtonText: "Apply"
    contentMargins: dp(8)

    positiveButtonSize: 13
    negativeButtonSize: 13

    onRejected:{
        settings.setAdvancedMode(advmode)
        settings.setFlickableGridMode(flickableGridMode)
    }

    onClosed: {
        advmode = Qt.binding(function() {return settings.getAdvancedMode()})
        flickableGridMode = Qt.binding(function() {return settings.getFlickableGridMode()})
    }

    // Should works as follow
    //positiveButtonText: "Apply"
    //negativeButtonText: "Cancel"

    Label {
        id: titleLabel

        anchors.left: parent.left
        anchors.leftMargin: dp(15)
        verticalAlignment: Text.AlignVCenter
        height: dp(50)
        wrapMode: Text.Wrap
        text: "Settings"
        style: "title"
        color: Theme.accentColor
    }

    Item{
        width: main.width < dp(900) ? main.width - dp(100) : dp(600)
        height: main.width < dp(450) ? main.width - dp(100) : dp(300)

        Column{
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.width/4

            ListItem.Standard {
                text: "General"

                selected: tabView.currentIndex === 0
                onClicked: tabView.currentIndex = 0
            }
        }

        QtControls.TabView {
            id: tabView
            frameVisible: false
            tabsVisible: false
            anchors.leftMargin: parent.width/4
            anchors.fill: parent
            QtControls.Tab {
                title: "General"
                Column {
                    anchors.fill: parent

                    ListItem.Subtitled {
                        text: "Start Sonet on system start"
                        height: dp(55)
                        interactive: false
                        secondaryItem: Switch {
                            id: switch2
                            anchors.verticalCenter: parent.verticalCenter
                            enabled: false
                        }
                    }

                    ListItem.Subtitled {
                        text: "Scrollable desktop"
                        height: dp(55)
                        secondaryItem: Switch {
                            id: switch4
                            anchors.verticalCenter: parent.verticalCenter
                            checked: flickableGridMode

                            onClicked: {
                                flickableGridMode = switch4.checked
                                switch4.checked = Qt.binding(function() {return flickableGridMode})
                            }
                        }

                        onClicked: {
                            switch4.checked = !switch4.checked
                            flickableGridMode = switch4.checked
                            switch4.checked = Qt.binding(function() {return flickableGridMode})
                        }
                    }

                    ListItem.Subtitled {
                        text: "Advanced mode"
                        height: dp(55)
                        secondaryItem: Switch {
                            id: switch3
                            anchors.verticalCenter: parent.verticalCenter
                            checked: advmode

                            onClicked: {
                                advmode = switch3.checked
                                switch3.checked = Qt.binding(function() {return advmode})
                            }
                        }

                        onClicked: {
                            switch3.checked = !switch3.checked
                            advmode = switch3.checked
                            switch3.checked = Qt.binding(function() {return advmode})
                        }
                    }
                }
            }
        }
    }
}
