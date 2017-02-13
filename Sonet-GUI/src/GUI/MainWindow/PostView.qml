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

import QtQuick.Layouts 1.3

import Sonet 1.0

View{
    id: post
    width: parent.width
    state: "hide"
    elevation: 1
    radius: 10

    states:[
        State {
            name: "hide";
            PropertyChanges { target: post; height: postSection.height}
            PropertyChanges { target: commentsSection; visible: false}
        },
        State {
            name: "show";
            PropertyChanges { target: post; height: postSection.height + commentsSection.height }
            PropertyChanges { target: commentsSection; visible: true}
        }
    ]

    transitions:[
        Transition {
            from: "hide"; to: "show"
            NumberAnimation { target: post; property: "height"; duration: MaterialAnimation.pageTransitionDuration/2 }
        }
    ]

    Rectangle{
        id: postSection
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        height: postHeader.height + (postContent.height + dp(10)) + postFooter.height
        z: 2
        radius: 10

        Item{
            id: postHeader
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.leftMargin: dp(25)
            anchors.rightMargin: dp(25)
            height: dp(50)


            Canvas {
                id: canvas
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.margins: dp(5)
                anchors.leftMargin: dp(0)
                width: height

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
            }

            Text {
                id: nameText
                anchors.left: canvas.right
                anchors.leftMargin: dp(15)
                height: parent.height
                text: friendname
                color: Palette.colors["grey"]["800"]
                font.family: "Roboto"
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                font.pixelSize: 14
            }

            Text {
                id: dateText
                anchors.right: parent.right
                height: parent.height
                text: date
                color: Theme.light.hintColor
                font.family: "Roboto"
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                font.pixelSize: 12
            }
        }

        Item{
            id: postContent
            anchors.top: postHeader.bottom
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.leftMargin: dp(25)
            anchors.rightMargin: dp(25)
            anchors.bottomMargin: dp(10)
            height: textMsg.implicitHeight

            TextEdit {
                id: textMsg
                text: contentmsg
                anchors.top: parent.top
                anchors.topMargin: dp(5)
                anchors.left: parent.left
                anchors.right: parent.right
                textFormat: Text.RichText
                wrapMode: Text.WordWrap
                color: Palette.colors["grey"]["900"]
                readOnly: true
                selectByMouse: true
                selectionColor: Theme.accentColor

                font {
                    family: "Roboto"
                    pixelSize: 14 * Units.dp
                }
            }
        }

        Item{
            id: postFooter
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.leftMargin: dp(25)
            anchors.rightMargin: dp(25)
            height: dp(50)

            states:[
                State {
                    name: "small"; when: postFooter.width < dp(400)
                    PropertyChanges { target: s1st; width: icon1.width + count1.implicitWidth + dp(5) }
                    PropertyChanges { target: s2nd; width: icon2.width + count2.implicitWidth + dp(5) }
                    PropertyChanges { target: name3; visible: false }
                    PropertyChanges { target: s3rd; width: icon3.width + count3.implicitWidth + dp(5) }
                },
                State {
                    name: "medium"; when: postFooter.width >= dp(400)
                    PropertyChanges { target: s1st; width: icon1.width + count1.implicitWidth + dp(5) }
                    PropertyChanges { target: s2nd; width: icon2.width + count2.implicitWidth + dp(5) }
                    PropertyChanges { target: name3; visible: true }
                    PropertyChanges { target: s3rd; width: icon3.width + name3.implicitWidth + count3.implicitWidth + dp(8) }
                }
            ]

            Item{
                id: s1st
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: icon1.width + count1.implicitWidth + dp(5)
                Icon {
                    id: icon1
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter

                    name: "awesome/thumbs_up"
                    visible: true
                    color: Theme.light.hintColor

                    size: 21 * Units.dp
                }

                Text {
                    id: count1
                    anchors.left: icon1.right
                    anchors.leftMargin: dp(3)
                    height: parent.height
                    text: voteup
                    color: Theme.light.hintColor
                    font.family: "Roboto"
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                    font.pixelSize: 12
                }

                MouseArea{
                    anchors.fill: parent

                    onClicked: wallPostModel.vote(msgid, grpid, true)
                }
            }

            Item{
                id: s2nd
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: s1st.right
                anchors.leftMargin: dp(8)

                width: icon2.width + count2.implicitWidth + dp(5)
                Icon {
                    id: icon2
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter

                    name: "awesome/thumbs_down"
                    visible: true
                    color: Theme.light.hintColor

                    size: 21 * Units.dp
                }

                Text {
                    id: count2
                    anchors.left: icon2.right
                    anchors.leftMargin: dp(3)
                    height: parent.height
                    text: votedown
                    color: Theme.light.hintColor
                    font.family: "Roboto"
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                    font.pixelSize: 12
                }

                MouseArea{
                    anchors.fill: parent

                    onClicked: wallPostModel.vote(msgid, grpid, false)
                }
            }

            Item{
                id: s3rd
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                width: icon3.width + name3.implicitWidth + count3.implicitWidth + dp(8)
                Icon {
                    id: icon3
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter

                    name: "awesome/comment"
                    visible: true
                    color: Theme.light.hintColor

                    size: 21 * Units.dp
                }

                Text {
                    id: count3
                    anchors.left: icon3.right
                    anchors.leftMargin: dp(5)
                    height: parent.height
                    text: qcomments
                    color: Theme.light.hintColor
                    font.family: "Roboto"
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                    font.pixelSize: 12
                }

                Text {
                    id: name3
                    anchors.left: count3.right
                    anchors.leftMargin: dp(3)
                    height: parent.height
                    text: {
                        if(qcomments === "" || qcomments === "1")
                            qsTr("Comment")
                        else
                            qsTr("Comments")
                    }
                    color: Theme.light.hintColor
                    font.family: "Roboto"
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                    font.pixelSize: 12
                }

                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        if(post.state === "show")
                        {
                            post.state = "hide"
                            return;
                        }

                        wallCommentModel.loadPostData(msgid, grpid);
                        post.state = "show"
                    }
                }
            }
        }
    }


    Item{
        id: commentsSection
        height: line.height + commentsHeader.height + commentsContent.height

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.leftMargin: dp(25)
        anchors.rightMargin: dp(25)

        visible: false

        z: 1

        Rectangle{
            id: line
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: dp(1)
            color: Palette.colors["grey"]["200"]
        }

        Item{
            id: commentsHeader
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: dp(1)
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

            TextField {
                id: msgBox
                placeholderText: "Comment it!"
                anchors.top: parent.top
                anchors.topMargin: dp(12)
                anchors.leftMargin: dp(12)
                anchors.left: canvas2.right
                anchors.right: parent.right

                font {
                    pixelSize: 15 * Units.dp
                }

                showBorder: false

                onAccepted: {
                    wallCommentModel.createComment(msgBox.text)
                    msgBox.text = ""
                }
            }
        }
        Item{
            id: commentsContent
            anchors.top: commentsHeader.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            height: listView.height

            ListView{
                id: listView
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: contentHeight
                clip: true
                model:  WallCommentModel{id: wallCommentModel}
                delegate: CommentDelegate{}
            }
        }
    }
}
