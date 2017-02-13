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
#include "wallcommentmodel.h"

//libretroshare
#include <retroshare/rsidentity.h>

#define GXSCOMMENTS_LOADTHREAD		1

using namespace RsWall;

/// //////////////////////////
/// \brief getHumanReadableDuration
/// \param seconds
/// \return QString of time
/// Function was got from retroshare-gui IdDialog.cpp
/// Created by Robert Fernie
/// ////////////////////////////
static QString getHumanReadableDuration(uint32_t seconds)
{
    if(seconds < 60)
        return QString(QObject::tr("%1 seconds ago")).arg(seconds) ;
    else if(seconds < 120)
        return QString(QObject::tr("%1 minute ago")).arg(seconds/60) ;
    else if(seconds < 3600)
        return QString(QObject::tr("%1 minutes ago")).arg(seconds/60) ;
    else if(seconds < 7200)
        return QString(QObject::tr("%1 hour ago")).arg(seconds/3600) ;
    else if(seconds < 24*3600)
        return QString(QObject::tr("%1 hours ago")).arg(seconds/3600) ;
    else if(seconds < 2*24*3600)
        return QString(QObject::tr("%1 day ago")).arg(seconds/86400) ;
    else
        return QString(QObject::tr("%1 days ago")).arg(seconds/86400) ;
}

WallCommentModel::WallCommentModel(QObject *parent):QAbstractListModel(parent)
{
    mTokenQueue = new TokenQueue(rsWall->getTokenService(), this);
    mCommentService = rsWall;
}

WallCommentModel::~WallCommentModel()
{
    delete mTokenQueue;
}

void WallCommentModel::updateDisplay()
{
    updateData();
    beginResetModel();
    endResetModel();
}

void WallCommentModel::loadPostData(QString msg, QString grp)
{
    comments.clear();

    RsGxsGroupId tempGrpId(grp.toStdString());
    RsGxsMessageId tempMsgId(msg.toStdString());
    mThreadId.first = grpId = tempGrpId;
    mThreadId.second = msgId = tempMsgId;

    updateDisplay();
}

void WallCommentModel::updateData()
{
    comments.clear();
    RsTokReqOptions opts;
    opts.mReqType = GXS_REQUEST_TYPE_MSG_RELATED_DATA;
    opts.mOptions = RS_TOKREQOPT_MSG_THREAD | RS_TOKREQOPT_MSG_LATEST;

    std::vector<RsGxsGrpMsgIdPair> msgIds;
    msgIds.push_back(mThreadId);

    uint32_t token;
    mTokenQueue->requestMsgRelatedInfo(token, RS_TOKREQ_ANSTYPE_DATA, opts, msgIds, GXSCOMMENTS_LOADTHREAD);
    waitForTokenOrTimeout(token, rsWall->getTokenService());
    mCommentService->getRelatedComments(token, comments);
}

void WallCommentModel::createComment(QString com)
{
    RsGxsComment comment;

    comment.mComment = com.toStdString();
    comment.mMeta.mParentId = mThreadId.second;
    comment.mMeta.mGroupId = mThreadId.first;
    comment.mMeta.mThreadId = mThreadId.second;

    std::list<RsGxsId> own_identities;
    rsIdentity->getOwnIds(own_identities);
    std::list<RsGxsId>::const_iterator gxsIt = own_identities.begin();
    RsGxsId authorId = *gxsIt;
    if(authorId.isNull())
        return;

    comment.mMeta.mAuthorId = authorId;

    uint32_t token;
    mCommentService->createComment(token, comment);
    mTokenQueue->queueRequest(token, TOKENREQ_MSGINFO, RS_TOKREQ_ANSTYPE_ACK, 0);
}

int WallCommentModel::rowCount(const QModelIndex & parent) const
{
    return comments.size();
}

QVariant WallCommentModel::data(const QModelIndex & index, int role) const
{
    int idx = index.row();
    if (idx < 0 || idx >= comments.size())
        return QVariant("Something goes wrong... :(");

    RsGxsComment com;
    int i = 0;

    for(std::vector<RsGxsComment>::const_iterator vit = comments.begin(); vit != comments.end(); ++vit)
    {
        if ( idx == i)
        {
            com = *vit;
            break;
        }
      ++i;
    }

    if (role == ContentMsgRole)
        return QString::fromStdString(com.mComment);

    if (role == FriendNameRole)
    {
        RsIdentityDetails details;
        rsIdentity->getIdDetails(com.mMeta.mAuthorId, details);
        return QString::fromStdString(details.mNickname.c_str());
    }

    if (role == GxsIdRole)
        return QString::fromStdString(com.mMeta.mAuthorId.toStdString());

    if (role == PostDateRole)
    {
        time_t now = time(NULL) ;
        return getHumanReadableDuration(now - com.mMeta.mPublishTs);
    }

    if (role == UpVotesRole)
        return QString::number(com.mUpVotes);

    if (role == DownVotesRole)
        return QString::number(com.mDownVotes);

    if (role == MsgIdRole)
        return QString::fromStdString(com.mMeta.mMsgId.toStdString());

    if (role == GrpIdRole)
        return QString::fromStdString(com.mMeta.mGroupId.toStdString());
}

QHash<int, QByteArray> WallCommentModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[FriendNameRole] = "friendname";
    roles[ContentMsgRole] = "contentmsg";
    roles[PostDateRole] = "date";
    roles[UpVotesRole] = "upvotes";
    roles[DownVotesRole] = "downvotes";
    roles[MsgIdRole] = "commsgid";
    roles[GrpIdRole] = "comgrpid";
    roles[GxsIdRole] = "comgxsid";

    return roles;
}
