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
#include "popup.h"

//std
#include "iostream"

//Qt
#include <QtWidgets>
#include <QQmlContext>
#include <QQmlEngine>
#include <QSound>

//Sonet-GUI
#include "Bridge/SoundManager.h"
#include "Bridge/VOIP/vnotify.h"

PopUp::PopUp(QWidget *parent) : QQuickWidget(parent)
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    setClearColor(Qt::transparent);
    setAttribute(Qt::WA_TranslucentBackground);

    QObject::connect(this->engine(),SIGNAL(quit()),this, SLOT(destroyWin())) ;
    QQmlEngine *engine = this->engine();
    QPM_INIT((*engine));
    rootContext()->setContextProperty("popup", this);

    qsound = soundManager->playEvent(SOUND_RINGING);
    if(qsound->fileName() != "")
    {
        qsound->setLoops(QSound::Infinite);
        qsound->play();
    }
}

void PopUp::destroyWin()
{
    qsound->stop();

    QObject::disconnect(VNotify::getInstance(), SIGNAL(voipHangUpReceived(const RsPeerId&,int)), this, SLOT(hangupReceived(RsPeerId, int)));
    this->destroy();
}

void PopUp::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        move(event->globalPos() - dragPosition);
        event->accept();
    }
}

void PopUp::pressed(int x, int y)
{
    dragPosition = QPoint(x, y);
}

void PopUp::setPeerId(QString peerId)
{
    rsPeerId = peerId;
}

void PopUp::hangupReceived(RsPeerId peerId, int flags)
{
    if(rsPeerId == QString::fromStdString(peerId.toStdString()))
        this->destroyWin();
}
