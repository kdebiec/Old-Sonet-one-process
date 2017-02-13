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
#ifndef STATUSCOLOR_H
#define STATUSCOLOR_H

//Qt
#include <QObject>
#include <QColor>

//Sonet-GUI
#include "notifytxt.h"

class StatusColor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString rspgp MEMBER rspgp NOTIFY rspgpChanged)
    Q_PROPERTY(QColor color MEMBER color NOTIFY colorChanged)
public:
    StatusColor(QObject *parent = 0);

signals:
    void colorChanged();
    void rspgpChanged();

public slots:
    void updateStatus();

private slots:
    void rspgpSlot();

private:
    QString rspgp;
    QColor color;

    RsPgpId entry;
};

#endif // STATUSCOLOR_H
