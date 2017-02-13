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
#ifndef WALLPOSTMODEL_H
#define WALLPOSTMODEL_H

//Qt
#include <QAbstractListModel>

//libretroshare
#include <services/wall/p3wallservice.h>

//Sonet-GUI
#include "util/TokenQueue.h"

using namespace RsWall;

class WallPostModel : public QAbstractListModel, public TokenResponse
{
    Q_OBJECT
public:
    enum WallPostRoles{
        FriendNameRole,
        ContentMsgRole,
        PostDateRole,
        VoteUpRole,
        VoteDownRole,
        CommentRole,
        MsgIdRole,
        GrpIdRole
    };

    static WallPostModel *Create ();
    static void Destroy();
    static WallPostModel *getInstance ();

    ~WallPostModel();

    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role) const;

    void loadRequest(const TokenQueue*, const TokenRequest&){}

public slots:

    void loadWall(QString rs);
    void setMyWall();
    void updateDisplay();
    bool createPost(QString qtmsg);

    void vote(QString msgId, QString grpId, bool up);

protected:

    virtual QHash<int, QByteArray> roleNames() const;

private:
    explicit WallPostModel(QObject *parent = 0);
    static WallPostModel *_instance;

    void updateData();

    RsGxsId _AuthorId;
    std::vector<PostMsg> mPostMsgs;
    TokenQueue *mWallQueue;
};

bool waitForTokenOrTimeout(uint32_t token, RsTokenService* tokenService);

#endif // WALLPOSTMODEL_H
