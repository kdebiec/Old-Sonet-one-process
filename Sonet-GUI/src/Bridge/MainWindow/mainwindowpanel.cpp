/****************************************************************
 *  This file is part of Sonet.
 *  Sonet is distributed under the following license:
 *
 *  Copyright (c) 2015: Deimos
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
/* File is originally from https://github.com/deimos1877/BorderlessWindow */
#include "mainwindowpanel.h"

#include <windows.h>
#include <windowsx.h>

//Qt
#include <QApplication>
#include <QQmlEngine>
#include <QQmlContext>

//Sonet-GUI
#include "Bridge/Chat/chatmsgmodel.h"
#include "Bridge/VOIP/voip.h"
#include "Bridge/wallcommentmodel.h"
#include "Bridge/ssettings.h"
#include "util/imageprovider.h"
#include "Bridge/Profiles/myprofile.h"
#include "Bridge/friendlist.h"
#include "Bridge/pgplistmodel.h"
#include "Bridge/Chat/chathandler.h"
#include "Bridge/VOIP/voiphandler.h"
#include "Bridge/wallpostmodel.h"
#include "Bridge/gxsidmodel.h"
#include "Bridge/msgstore.h"
#include "Bridge/statuscolor.h"
#include "Bridge/SoundManager.h"

MainWindowPanel::MainWindowPanel( HWND hWnd ) : QWinView(hWnd)
{
    windowHandle = hWnd;
    setObjectName( "mainWindowPanel" );

    QObject::connect(NotifyTxt::getInstance(), SIGNAL(newMessage(bool)), this, SLOT(windowAlert(bool)));
    QObject::connect(NotifyTxt::getInstance(), SIGNAL(newMessage(bool)), this, SLOT(playNewMessage(bool)));

    this->setResizeMode(QQuickView::SizeRootObjectToView);

    QQmlEngine *engine = this->engine();
    engine->addImageProvider(QLatin1String("avatar"), ImageProvider::getInstance());
    QObject::connect(engine,SIGNAL(quit()),qApp, SLOT(quit()));
    QPM_INIT((*engine));

    QQmlContext *ctxt = this->rootContext();
    ctxt->setContextProperty("view", this);
    ctxt->setContextProperty("qMainPanel", this);
    ctxt->setContextProperty("cursor", this);
    ctxt->setContextProperty("control", this);

    ctxt->setContextProperty("settings", SSettings::getInstance());

    ctxt->setContextProperty("msgStore", MsgStore::getInstance());
    ctxt->setContextProperty("myProfile", MyProfile::getInstance());
    ctxt->setContextProperty("chatHandler", ChatHandler::getInstance());

    ctxt->setContextProperty("friendList", FriendList::getInstance());
    ctxt->setContextProperty("pgpListModel", PGPListModel::getInstance());
    ctxt->setContextProperty("wallPostModel", WallPostModel::getInstance());

    ctxt->setContextProperty("gxsIdModel", GxsIdModel::getInstance());

    ctxt->setContextProperty("voipHandler", VOIPHandler::getInstance());

    qmlRegisterType<ChatMsgModel>("Sonet", 1, 0, "ChatMsgModel");
    qmlRegisterType<VOIP>("Sonet", 1, 0, "VOIP");
    qmlRegisterType<WallCommentModel>("Sonet", 1, 0, "WallCommentModel");
    qmlRegisterType<StatusColor>("Sonet", 1, 0, "StatusColor");

    this->setSource(QUrl("qrc:/Borderless.qml"));
    show();
}

// Button events
void MainWindowPanel::pushButtonMinimizeClicked()
{
    ShowWindow( parentWindow(), SW_MINIMIZE );
}

void MainWindowPanel::pushButtonMaximizeClicked()
{
    WINDOWPLACEMENT wp;
    wp.length = sizeof( WINDOWPLACEMENT );
    GetWindowPlacement( parentWindow(), &wp );
    if ( wp.showCmd == SW_MAXIMIZE )
        ShowWindow( parentWindow(), SW_RESTORE );
    else
        ShowWindow( parentWindow(), SW_MAXIMIZE );
}

void MainWindowPanel::pushButtonCloseClicked()
{
    PostQuitMessage(0);
}

void MainWindowPanel::mouseLPressed()
{
    ReleaseCapture();
    SendMessage( windowHandle, WM_NCLBUTTONDOWN, HTCAPTION, 0 );
}

void MainWindowPanel::hide()
{
    ShowWindow( parentWindow(), SW_HIDE );
}

void MainWindowPanel::changeCursor(int cursorShape)
{
    this->setCursor(Qt::CursorShape(cursorShape));
}

void MainWindowPanel::resizeWin(int x, int y, bool changeposx, bool changeposy)
{
    WINDOWPLACEMENT wp;
    wp.length = sizeof( WINDOWPLACEMENT );
    GetWindowPlacement( windowHandle, &wp );
    if ( wp.showCmd != SW_MAXIMIZE )
    {
        RECT winrect;
        GetWindowRect( windowHandle, &winrect );
        long width = winrect.right - winrect.left;
        long height = winrect.bottom - winrect.top;

        if(changeposx && changeposy)
            SetWindowPos( windowHandle, 0, (winrect.left+x), (winrect.top+y), (width-x), (height-y), SWP_NOREDRAW );
        else if(changeposx && !changeposy)
            SetWindowPos( windowHandle, 0, (winrect.left+x), winrect.top, (width-x), (height+y), SWP_NOREDRAW );
        else if(!changeposx && changeposy)
            SetWindowPos( windowHandle, 0, winrect.left, (winrect.top+y), (width+x), (height-y), SWP_NOREDRAW );
        else if(!changeposx && !changeposy)
            SetWindowPos( windowHandle, 0, winrect.left, winrect.top, (width+x), (height+y), SWP_NOREDRAW );
    }
}

void MainWindowPanel::windowAlert(bool incoming)
{
    if(incoming && (GetActiveWindow() != windowHandle))
        FlashWindow(windowHandle, true);
}

void MainWindowPanel::playNewMessage(bool incoming)
{
    if((status() == Ready) && (GetActiveWindow() != windowHandle))
    {
        if(incoming)
            soundManager->play(SOUND_MESSAGE_RECEIVED);
    }
}

void MainWindowPanel::notifyEnable()
{
    NotifyTxt::getInstance()->enable();
    GxsIdModel::getInstance()->updateDisplay();
}
