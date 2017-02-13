/*
 * QML Material - An application framework implementing Material Design.
 *
 * Copyright (C) 2015-2016 Michael Spencer <sonrisesoftware@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.4
import Material 0.3
import Material.Extras 0.1

/*!
   \qmltype OverlayView
   \inqmlmodule Material

   \brief A view that pops out of the content to display as an overlay.
 */
PopupBase {
    id: overlay

    overlayLayer: "dialogOverlayLayer"
    overlayColor: Qt.rgba(0, 0, 0, 0.3)

    visible: transitionOpacity > 0
    state: showing ? "visible" : "hidden"

    x: (parent.width - width)/2
    y: (parent.height - height)/2

    property alias transitionOpacity: shadow.opacity

    property bool transitionShowing: false
    property int radiusOnStart: 0

    states: [
        State {
            name: "hidden"

            PropertyChanges {
                target: overlay
                x: sourceView ? sourceView.mapToItem(overlay.parent, 0, 0).x : 0
                y: sourceView ? sourceView.mapToItem(overlay.parent, 0, 0).y : 0
                width: sourceView ? sourceView.width : 0
                height: sourceView ? sourceView.height : 0
            }
        }
    ]

    transitions: Transition {
        from: "*"; to: "*"

        NumberAnimation {
            target: overlay
            properties: "x,y,width,height"
            duration: 300; easing.type: Easing.InOutQuad
        }
    }

    property Item sourceView

    function open(sourceView) {
        overlay.sourceView = sourceView;

        parent = Utils.findRootChild(overlay, overlayLayer)
        showing = true
        transitionShowing = true
        forceActiveFocus()
        parent.currentOverlay = overlay
        shadow.radius = 0

        opened()
    }

    function close() {
        showing = false
        transitionShowing = false
        parent.currentOverlay = null
        sourceView = null
        shadow.radius = radiusOnStart

        closed()
    }

    View {
        id: shadow

        anchors.fill: parent
        opacity: showing ? 1 : 0
        elevation: 5
        radius: radiusOnStart

        Behavior on radius{
            NumberAnimation {
                duration: 300; easing.type: Easing.InOutQuad
            }
        }

        Behavior on opacity {
            NumberAnimation {
                duration: 300; easing.type: Easing.InOutQuad
            }
        }
    }
}
