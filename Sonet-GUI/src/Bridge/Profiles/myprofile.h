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
#ifndef MYPROFILE_H
#define MYPROFILE_H

//Qt
#include <QObject>
#include <QUrl>
#include <QTimer>

//libretroshare
#include <retroshare/rspeers.h>
#include <retroshare/rsidentity.h>
#include <retroshare/rstypes.h>
#include <retroshare/rsmsgs.h>
#include <retroshare/rsservicecontrol.h>

//Sonet-GUI
#include "util/TokenQueue.h"
#include "notifytxt.h"

class RsGixs ;

class MyProfile : public QObject, public TokenResponse
{
    Q_OBJECT

public:
    static MyProfile *Create ();
    static void Destroy();
    static MyProfile *getInstance ();

    ~MyProfile();

    void loadRequest(const TokenQueue*, const TokenRequest&){}

signals:
    void firstIdentityCreated();
    void firstIdentityCancelled();

public slots:
    void setAvatar(/*QString gxs*/); // This function will be working only if
                                     // have just one identity.
    void setWallBg();

    QString showMyCert();

    QString getNickBaseIdentity();
    QString getPreferredGxs();

    bool haveIdentity();
    bool addIdentity(QString Nickname, QString src); // Creating new identity

    void changeStatus();
    QColor getStatus();

    void setStatusMsg(QString msg);
    QString getStatusMsg();

    void createWall();
    void firstIdentityRejected();

private:
    explicit MyProfile(QObject *parent = 0);
    static MyProfile *_instance;

    std::list<RsGxsId> own_identities ;
    RsGixs *mGixs ;

    RsGxsIdGroup mEditGroup;
    RsGxsGroupId mGroupId;

    TokenQueue *mIdQueue;
    TokenQueue *mWallQueue;

    QTimer* qTimer;
};

#endif // MYPROFILE_H
