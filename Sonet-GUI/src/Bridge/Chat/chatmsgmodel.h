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
#ifndef CHATMSGMODEL_H
#define CHATMSGMODEL_H

//Qt
#include <QObject>
#include <QAbstractListModel>

//Sonet-GUI
#include "notifytxt.h"
#include "chat.h"

class ChatMsgModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString gxs MEMBER gxs NOTIFY gxsChanged)
    Q_PROPERTY(QString rspgp MEMBER rspgp NOTIFY rspgpChanged)

public:
    enum ChatRoles{
        ContentMessageRole,
        NameRole,
        SendTimeRole,
        RecvTimeRole,
        SideRole
    };

    explicit ChatMsgModel(QObject *parent = 0);
        ~ChatMsgModel();

    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role) const;

signals:
    void gxsChanged();
    void rspgpChanged();
    void updatedDisplay();

public slots:
    void updateDisplay();
    void updateDisplay(bool incoming);
    void setChatViaGxs(QString gxs);
    void setChatViaPgp(QString pgp);

protected:
    virtual QHash<int, QByteArray> roleNames() const;

private slots:
    void gxsSlot();
    void rspgpSlot();

private:
    QString gxs;
    QString rspgp;
    RsPgpId entry;
    Chat *mChat;
};

#endif // CHATMSGMODEL_H
