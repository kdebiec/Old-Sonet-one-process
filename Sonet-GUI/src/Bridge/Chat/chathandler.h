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
#ifndef CHATHANDLER_H
#define CHATHANDLER_H

//Qt
#include <QObject>

class ChatHandler : public QObject
{
    Q_OBJECT
public:
    static ChatHandler *Create ();
    static void Destroy();
    static ChatHandler *getInstance ();

public slots:
    void sendMsgViaPGP(QString rs, QString msg);
    void sendMsgViaGxs(QString gxs, QString msg);

private:
    ChatHandler(QObject *parent = 0) : QObject(parent){}
    static ChatHandler *_instance;
};

#endif // CHATHANDLER_H
