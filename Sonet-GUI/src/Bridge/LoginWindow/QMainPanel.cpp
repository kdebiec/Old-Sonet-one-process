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
#include "QMainPanel.h"

#include <windows.h>
#include <windowsx.h>

//Qt
#include <QApplication>
#include <QQmlEngine>
#include <QQmlContext>

//Sonet-GUI
#include "loginwindow.h"

QMainPanel::QMainPanel( HWND hWnd ) : QWinView(hWnd)
{
    windowHandle = hWnd;
    setObjectName( "qMainPanel" );

    this->setResizeMode(QQuickView::SizeRootObjectToView);
    QQmlEngine *engine = this->engine();
    QObject::connect(engine,SIGNAL(quit()),qApp, SLOT(quit()));
    QPM_INIT((*engine));

    genCert = new GenCert();

    QQmlContext *ctxt = this->rootContext();
    ctxt->setContextProperty("qMainPanel", this);
    ctxt->setContextProperty("loginWindow", LoginWindow::getInstance());
    ctxt->setContextProperty("genCert", genCert);
    this->setSource(QUrl("qrc:/borderless.qml"));
    show();
}

QMainPanel::~QMainPanel()
{
    if(genCert != NULL)
        delete genCert ;

    genCert = NULL ;
}

// Button events
void QMainPanel::pushButtonMinimizeClicked()
{
    ShowWindow( parentWindow(), SW_MINIMIZE );
}

void QMainPanel::pushButtonMaximizeClicked()
{
    WINDOWPLACEMENT wp;
    wp.length = sizeof( WINDOWPLACEMENT );
    GetWindowPlacement( parentWindow(), &wp );
    if ( wp.showCmd == SW_MAXIMIZE )
        ShowWindow( parentWindow(), SW_RESTORE );
    else
        ShowWindow( parentWindow(), SW_MAXIMIZE );
}

void QMainPanel::pushButtonCloseClicked()
{
    PostQuitMessage(0);
}

void QMainPanel::mouseLPressed()
{
    ReleaseCapture();
    SendMessage( windowHandle, WM_NCLBUTTONDOWN, HTCAPTION, 0 );
}
