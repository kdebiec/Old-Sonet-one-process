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

//Qt
#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

//Sonet-GUI
#include "Bridge/ssettings.h"
#include "Bridge/gxsidmodel.h"
#include "Bridge/wallpostmodel.h"
#include "Bridge/pgplistmodel.h"
#include "Bridge/Chat/chathandler.h"
#include "Bridge/VOIP/voiphandler.h"
#include "Bridge/Profiles/myprofile.h"
#include "Bridge/friendlist.h"
#include "Bridge/msgstore.h"
#include "util/imageprovider.h"
#include "util/screensize.h"

#ifndef BORDERLESS_MAINWINDOW
    //Qt
    #include <QQuickView>
    #include <QQmlContext>
    #include <QMetaType>

    //Sonet-GUI
    #include "rsqml_main.h"
    #include "notifytxt.h"
    #include "Bridge/Chat/chatmsgmodel.h"
    #include "Bridge/Chat/voip.h"
    #include "Bridge/wallcommentmodel.h"
    #include "Bridge/statuscolor.h"
    #include "util/cursorshape.h"
    #include "util/qquickviewhelper.h"

#endif
#ifdef BORDERLESS_MAINWINDOW
    //Sonet-GUI
    #include "Bridge/MainWindow/mainwindow.h"
#endif

int rsqml_main(int argc, char **argv)
{
    	QApplication app(argc, argv);
        QApplication::setQuitOnLastWindowClosed(false);

        QPixmap pixmap(32, 32);
        pixmap.fill(QColor("#4caf50"));
        QIcon icon(pixmap);

        /** Tray Icon Menu **/
        QMenu *trayMenu = new QMenu();
        QAction *quitAction = new QAction("Quit", trayMenu);
        QObject::connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
        trayMenu->addAction(quitAction);

        /** End of Icon Menu **/

        QSystemTrayIcon trayIcon(icon);
        trayIcon.setContextMenu(trayMenu);

        trayIcon.show();

    SSettings::Create();
    ImageProvider::Create();
    MsgStore::Create();
    GxsIdModel::Create();
    WallPostModel::Create();
    PGPListModel::Create();
    ChatHandler::Create();
    VOIPHandler::Create();
    MyProfile::Create();
    FriendList::Create();

#ifndef BORDERLESS_MAINWINDOW
    QQuickView *view = new QQuickView;
    view->setResizeMode(QQuickView::SizeRootObjectToView);

    QQmlEngine *engine = view->engine();
    engine->addImageProvider(QLatin1String("avatar"), ImageProvider::getInstance());
    QObject::connect(engine,SIGNAL(quit()),qApp, SLOT(quit())) ;
    QPM_INIT((*engine));

	QQmlContext *ctxt = view->rootContext();
    ctxt->setContextProperty("view", view);

    CursorShape cursor(view);
    ctxt->setContextProperty("cursor", &cursor);

    QQuickViewHelper helper(view);
    ctxt->setContextProperty("control", &helper);

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

    view->setSource(QUrl("qrc:/MainGUI.qml"));
    // Create window
    ScreenSize screenSize;
    view->setWidth(screenSize.width()/2);
    view->setHeight(screenSize.height()/2);
    view->setMinimumWidth(600);
    view->setMinimumHeight(300);
    view->show();

    QObject::connect(NotifyTxt::getInstance(), SIGNAL(newMessage(bool)), &helper, SLOT(alert(bool)));
    QObject::connect(&trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), &helper, SLOT(showViaSystemTrayIcon(QSystemTrayIcon::ActivationReason)));
#endif
#ifdef BORDERLESS_MAINWINDOW
    // Background color
    HBRUSH windowBackground = CreateSolidBrush( RGB( 255, 255, 255 ) );

    // Create window
    ScreenSize screenSize;
    MainWindow window( &app, windowBackground, screenSize.width()/4, screenSize.height()/4, screenSize.width()/2, screenSize.height()/2 );
    window.setMinimumSize(600, 300);

    QObject::connect(&trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), &window, SLOT(showViaSystemTrayIcon(QSystemTrayIcon::ActivationReason)));
#endif

    return app.exec();
}



