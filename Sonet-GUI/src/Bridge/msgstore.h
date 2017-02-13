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
#ifndef MSGSTORE_H
#define MSGSTORE_H

//Qt
#include <QObject>

//Sonet-GUI
#include "notifytxt.h"

class MsgStore : public QObject
{
    Q_OBJECT
public:
    static MsgStore *Create ();
    static void Destroy();
    static MsgStore *getInstance ();

    ~MsgStore();

    std::vector<MsgPreview>* getStoredMsgs() const;

public slots:
    void cleanStoredMsgForPGP(QString rspgp);
    void storeMsg(MsgPreview &msg);
    void storeMsgs(std::vector<MsgPreview> &msgs);

signals:
    void msgReceived();
    void msgStoreCleaned();

private:
    MsgStore(QObject *parent = 0);
    static MsgStore *_instance;

    std::vector<MsgPreview> *storedMsg;
};

#endif // MSGSTORE_H
