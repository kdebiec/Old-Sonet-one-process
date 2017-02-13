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
#ifndef GXSIDMODEL_H
#define GXSIDMODEL_H

//Qt
#include <QAbstractListModel>
#include <QTimer>

//libretroshare
#include <retroshare/rsidentity.h>

//Sonet-GUI
#include "util/TokenQueue.h"

class GxsIdModel : public QAbstractListModel, public TokenResponse
{
    Q_OBJECT
public:
    enum FriendsRoles{
        FriendNameRole,
        StatusColorRole,
        StatusMsgRole,
        Index,
        GxsIdRole,
        LastSeenRole,
        MsgCountRole,
        RsPgpIdRole
    };
    static GxsIdModel *Create ();
    static void Destroy();
    static GxsIdModel *getInstance ();

    ~GxsIdModel();

    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    void loadRequest(const TokenQueue*, const TokenRequest&){}

public slots:
    void updateDisplay();
    void updateDisplaySize();

    void requestUpdate();
    void requestUpdate(QString, int);
    void requestUpdate(QString, QString);

    void loadRequest();

protected:

    virtual QHash<int, QByteArray> roleNames() const;

private:
    GxsIdModel(QObject *parent = 0);
    static GxsIdModel *_instance;

    void updateData();

    TokenQueue *mIdQueue;

    std::vector<RsGxsIdGroup> datavector;
    bool checked;

    bool requestedUpdate;
    QTimer *_timer ;
    QTimer *_timer2 ;
};

#endif // GXSIDMODEL_H
