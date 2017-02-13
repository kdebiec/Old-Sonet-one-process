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
#ifndef QQUICKVIEWHELPER_H
#define QQUICKVIEWHELPER_H

//Qt
#include <QQuickView>
#include <QSystemTrayIcon>

//Sonet-GUI
#include "notifytxt.h"
#include "Bridge/SoundManager.h"
#include "Bridge/gxsidmodel.h"

class QQuickViewHelper : public QObject
{
    Q_OBJECT
public:
    QQuickViewHelper(QQuickView *v, QObject *parent = 0) : QObject(parent), view(v){}
public slots:
    void showViaSystemTrayIcon(QSystemTrayIcon::ActivationReason reason)
    {
        if(reason == QSystemTrayIcon::Trigger)
            view->show();
    }

    void alert(bool incoming)
    {
        if(incoming)
        {
            view->alert(0);

            if((view->status() == QQuickView::Ready) && !view->isActive())
                soundManager->play(SOUND_MESSAGE_RECEIVED);
        }
    }

    void notifyEnable()
    {
        NotifyTxt::getInstance()->enable();
        GxsIdModel::getInstance()->updateDisplay();
    }

private:
    QQuickView *view;
};

#endif // QQUICKVIEWHELPER_H
