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
#ifndef WALLCOMMENTMODEL_H
#define WALLCOMMENTMODEL_H

//Qt
#include <QAbstractListModel>

//libretroshare
#include <retroshare/rsgxscommon.h>
#include <services/wall/rswall.h>

//Sonet-GUI
#include "util/TokenQueue.h"

bool waitForTokenOrTimeout(uint32_t token, RsTokenService* tokenService);

class WallCommentModel : public QAbstractListModel, public TokenResponse
{
    Q_OBJECT
public:
    enum WallPostRoles{
        FriendNameRole,
        ContentMsgRole,
        PostDateRole,
        UpVotesRole,
        DownVotesRole,
        MsgIdRole,
        GrpIdRole,
        GxsIdRole
    };

    WallCommentModel(QObject *parent = 0);
    ~WallCommentModel();

    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role) const;

    void loadRequest(const TokenQueue*, const TokenRequest&){}

public slots:
    void loadPostData(QString msg, QString grp);
    void createComment(QString com);
    void updateDisplay();

protected:

    virtual QHash<int, QByteArray> roleNames() const;

private:
    void updateData();
    std::vector<RsGxsComment> comments;

    RsGxsMessageId msgId;
    RsGxsGroupId grpId;
    RsGxsGrpMsgIdPair mThreadId;

    RsGxsCommentService *mCommentService;
    TokenQueue *mTokenQueue;
};

#endif // WALLCOMMENTMODEL_H
