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

//Sonet-GUI
#include "loginwindow_main.h"
#include "loginwindow.h"

#ifdef BORDERLESS_LOGIN
    //Sonet-GUI
    #include "borderlesswindow.h"
    #include "../../util/screensize.h"
#endif
#ifndef BORDERLESS_LOGIN
    //Qt
    #include <QQmlContext>
    #include <QQuickView>
    #include <QQmlEngine>
    #include <QObject>
    #include <QSize>

    //Sonet-GUI
    #include "../gencert.h"
#endif

int loginwindow_main(int argc, char **argv)
{
        QApplication app(argc, argv);

    QObject::connect(LoginWindow::getInstance(), SIGNAL(closeWindow()), &app,  SLOT(quit()), Qt::QueuedConnection); // This connection close main loop.
#ifndef BORDERLESS_LOGIN
    GenCert genCert;

    QQuickView*view = new QQuickView;
    view->setResizeMode(QQuickView::SizeRootObjectToView);

    view->setMaximumSize(QSize(410, 480));
    view->setMinimumSize(QSize(410, 480));

    QQmlEngine *engine = view->engine();
    QPM_INIT((*engine));

    QObject::connect(&app, SIGNAL(aboutToQuit()), view, SLOT(close())); // This connection kills loginwindow.

    QQmlContext *ctxt = view->rootContext();

    ctxt->setContextProperty("loginWindow", LoginWindow::getInstance());
    ctxt->setContextProperty("genCert", &genCert);

    view->setSource(QUrl("qrc:/bordered.qml"));
    view->show();
#endif
#ifdef BORDERLESS_LOGIN
    // Background color
    HBRUSH windowBackground = CreateSolidBrush( RGB( 255, 255, 255 ) );

    // Create window
    ScreenSize screenSize;
    BorderlessWindow window( &app, windowBackground, (screenSize.width()-400)/2, (screenSize.height()-470)/2, 400, 470 );
#endif

    return app.exec();
}
