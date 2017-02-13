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
#ifndef PGPLISTMODEL_H
#define PGPLISTMODEL_H

//Qt
#include <QAbstractListModel>

//libretroshare
#include <retroshare/rspeers.h>
#include <retroshare/rstypes.h>

class QTimer ;

class PGPListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum FriendsRoles{
        FriendNameRole,
        StatusRole,
        Index,
        RsPgpIdRole,
        StatusMsgRole,
        MsgCountRole
    };
    static PGPListModel *Create ();
    static void Destroy();
    static PGPListModel *getInstance ();

    ~PGPListModel();

    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

public slots:
    void updateDisplayData();
    void updateDisplaySize();

    void requestUpdate();
    void requestUpdate(QString, int);
    void requestUpdate(QString, QString);

    void loadRequest();

protected:
    virtual QHash<int, QByteArray> roleNames() const;

private:
    PGPListModel(QObject *parent = 0);
    static PGPListModel *_instance;

    std::list<RsPgpId> gpgFriends;
    RsPgpId ownId;

    bool requestedUpdate;
    QTimer *_timer ;
};

#endif // PGPLISTMODEL_H
