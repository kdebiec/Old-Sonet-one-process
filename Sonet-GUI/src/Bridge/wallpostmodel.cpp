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
#include "wallpostmodel.h"

//libretroshare
#include <retroshare/rsidentity.h>

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

/*static*/ WallPostModel *WallPostModel::_instance = NULL;

/*static*/ WallPostModel *WallPostModel::Create()
{
    if (_instance == NULL)
        _instance = new WallPostModel();

    return _instance;
}

/*static*/ void WallPostModel::Destroy()
{
    if(_instance != NULL)
        delete _instance ;

    _instance = NULL ;
}

/*static*/ WallPostModel *WallPostModel::getInstance()
{
    return _instance;
}

WallPostModel::WallPostModel(QObject *parent) : QAbstractListModel(parent)
{
    mWallQueue = new TokenQueue(rsWall->getTokenService(), this);

    std::list<RsGxsId> ids;
    rsIdentity->getOwnIds(ids);
    if(!ids.empty())
    {
        std::list<RsGxsId>::const_iterator gxsIt = ids.begin();
        _AuthorId = *gxsIt;

        updateData();
    }
}

WallPostModel::~WallPostModel()
{
    delete mWallQueue;
}

void WallPostModel::setMyWall()
{
    std::list<RsGxsId> ids;
    rsIdentity->getOwnIds(ids);
    if(!ids.empty())
    {
        std::list<RsGxsId>::const_iterator gxsIt = ids.begin();
        time_t start = time(NULL);
        bool ok = false;
        do
        {
            gxsIt = ids.begin();
            for (gxsIt; gxsIt != ids.end(); ++gxsIt)
            {
                RsIdentityDetails details;
                ok = rsIdentity->getIdDetails(*gxsIt, details);
                if(ok && !details.mNickname.empty())
                    break;
            }
#ifdef WINDOWS_SYS
            Sleep(500);
#else
            usleep(500*1000);
#endif
        } while(!ok && (time(NULL)< (start+10)));

        _AuthorId = *gxsIt;
    }
    updateData();
}

void WallPostModel::loadWall(QString rs)
{
    RsGxsId authorId(rs.toStdString());
    _AuthorId = authorId;

    uint32_t walltoken;
    RsTokReqOptions opts;
    opts.mReqType = GXS_REQUEST_TYPE_GROUP_DATA;
    mWallQueue->requestGroupInfo(walltoken, RS_TOKREQ_ANSTYPE_DATA, opts, 1);

    waitForTokenOrTimeout(walltoken, rsWall->getTokenService());

    std::vector<WallGroup> groups;
    if (!rsWall->getWallGroups(walltoken, groups))
        std::cerr << "WallPostModel::loadWall() Error getting WallGroups" << std::endl;

    for (std::vector<WallGroup>::iterator it = groups.begin() ; it != groups.end(); ++it)
    {
        if (it->mMeta.mAuthorId == authorId)
        {
            uint32_t token;
            rsWall->subscribeToGroup(token, it->mMeta.mGroupId, true);
        }
    }

    updateDisplay();
}

void WallPostModel::updateDisplay()
{
    updateData();
    beginResetModel();
    endResetModel();
}

void WallPostModel::vote(QString msgId, QString grpId, bool up)
{
    std::list<RsGxsId> own_identities ;
    rsIdentity->getOwnIds(own_identities);
    std::list<RsGxsId>::const_iterator gxsIt = own_identities.begin();
    RsGxsId author = *gxsIt;
    if(author.isNull())
        return;

    RsGxsVote vote;
    vote.mMeta.mGroupId = RsGxsGroupId(grpId.toStdString());
    vote.mMeta.mThreadId = RsGxsMessageId(msgId.toStdString());
    vote.mMeta.mParentId = RsGxsMessageId(msgId.toStdString());
    vote.mMeta.mAuthorId = author;

    if (up)
        vote.mVoteType = GXS_VOTE_UP;
    else
        vote.mVoteType = GXS_VOTE_DOWN;

    uint32_t token;
    rsWall->createVote(token, vote);
    mWallQueue->queueRequest(token, TOKENREQ_MSGINFO, RS_TOKREQ_ANSTYPE_ACK, 0);
}

bool WallPostModel::createPost(QString qtmsg)
{
    PostMsg msg;
    uint32_t walltoken;
    uint32_t posttoken;

    std::list<RsGxsId> own_identities;
    rsIdentity->getOwnIds(own_identities);
    std::list<RsGxsId>::const_iterator gxsIt = own_identities.begin();
    if(!own_identities.empty())
    {
        time_t start = time(NULL);
        // have to try to get the identity details multiple times, until they are cached
        bool ok  = false;
        do
        {
            gxsIt = own_identities.begin();
            for (gxsIt; gxsIt != own_identities.end(); ++gxsIt)
            {
                RsIdentityDetails details;
                ok = rsIdentity->getIdDetails(*gxsIt, details);
                if(ok && !details.mNickname.empty())
                    break;
            }
#ifdef WINDOWS_SYS
            Sleep(500);
#else
            usleep(500*1000);
#endif
        } while(!ok && (time(NULL)< (start+10)));
    }

    RsGxsId author = *gxsIt;
    if(author.isNull())
        return false;

    msg.mMeta.mAuthorId = author;

    RsTokReqOptions opts;
    opts.mReqType = GXS_REQUEST_TYPE_GROUP_DATA;
    mWallQueue->requestGroupInfo(walltoken, RS_TOKREQ_ANSTYPE_DATA, opts, 1);

    waitForTokenOrTimeout(walltoken, rsWall->getTokenService());

    std::vector<WallGroup> groups;
    if (!rsWall->getWallGroups(walltoken, groups))
        std::cerr << "WallPostModel::createPost() Error getting WallGroups" << std::endl;

    std::vector<WallGroup>::iterator wg = groups.begin();
    for (std::vector<WallGroup>::iterator it = groups.begin() ; it != groups.end(); ++it)
    {
        if (it->mMeta.mAuthorId == *gxsIt)
            *wg = *it;
    }

    RsGxsGroupId targetWall;
    targetWall = wg->mMeta.mGroupId;
    if(targetWall.isNull())
        return false;

    // use the group id to signal the wall the post should appear on
    msg.mMeta.mGroupId = targetWall;
    msg.mPostText = qtmsg.toUtf8().constData();
    rsWall->createPost(posttoken, msg);

    return true;
}

void WallPostModel::updateData()
{
    mPostMsgs.clear();

    RsTokReqOptions grpopts;
    grpopts.mReqType = GXS_REQUEST_TYPE_GROUP_DATA;
    uint32_t walltoken;
    mWallQueue->requestGroupInfo(walltoken, RS_TOKREQ_ANSTYPE_DATA, grpopts, 1);

    waitForTokenOrTimeout(walltoken, rsWall->getTokenService());

    std::vector<WallGroup> groups;
    if (!rsWall->getWallGroups(walltoken, groups))
        std::cerr << "WallPostModel::updateData() Error getting WallGroups" << std::endl;

    WallGroup wg;

    for (std::vector<WallGroup>::iterator it = groups.begin() ; it != groups.end(); ++it)
    {
        if (it->mMeta.mAuthorId == _AuthorId)
            wg = *it;
    }

    RsGxsGroupId targetWall = wg.mMeta.mGroupId;
    std::list<RsGxsGroupId> grpIds;
    grpIds.push_back(targetWall);

    RsTokReqOptions opts;
    opts.mReqType = GXS_REQUEST_TYPE_MSG_DATA;
    opts.mOptions = RS_TOKREQOPT_MSG_THREAD;

    uint32_t msgtoken;
    rsWall->getTokenService()->requestMsgInfo(msgtoken, RS_TOKREQ_ANSTYPE_DATA, opts, grpIds);
    waitForTokenOrTimeout(msgtoken, rsWall->getTokenService());

    if(!rsWall->getPostData(msgtoken, mPostMsgs))
    {
        std::cerr << "WallPostModel::updateData() Error getting PostMsgs" << std::endl;
        return;
    }
}

int WallPostModel::rowCount(const QModelIndex&) const
{
    return mPostMsgs.size();
}

QVariant WallPostModel::data(const QModelIndex & index, int role) const
{
    int idx = index.row();
    if (idx < 0 || idx >= mPostMsgs.size())
        return QVariant("Something goes wrong... :(");

    PostMsg pm;
    int i = 0;

    for(std::vector<PostMsg>::const_reverse_iterator vit = mPostMsgs.rbegin(); vit != mPostMsgs.rend(); ++vit)
    {
        if ( idx == i)
        {
            pm = *vit;
#ifdef PGPLISTMODEL_DEBUG
            std::cerr << "WallPostModel::data finded gpgFriends" << std::endl;
#endif
            break;
        }
      ++i;
    }

    pm.calculateScores(NULL);
    if (role == ContentMsgRole)
        return QString::fromStdString(pm.mPostText);

    if (role == FriendNameRole)
    {
        RsIdentityDetails details;
        rsIdentity->getIdDetails(pm.mMeta.mAuthorId, details);
        return QString::fromStdString(details.mNickname.c_str());
    }

    if (role == PostDateRole)
    {
        time_t now = time(NULL) ;
        return getHumanReadableDuration(now - pm.mMeta.mPublishTs);
    }

    if (role == CommentRole)
    {
        if (pm.mComments)
            return QString::number(pm.mComments);
        else
            return QString("");
    }

    if (role == VoteUpRole)
        return QString::number(pm.mUpVotes);

    if (role == VoteDownRole)
        return QString::number(pm.mDownVotes);

    if (role == MsgIdRole)
        return QString::fromStdString(pm.mMeta.mMsgId.toStdString());

    if (role == GrpIdRole)
        return QString::fromStdString(pm.mMeta.mGroupId.toStdString());
}

QHash<int, QByteArray> WallPostModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[FriendNameRole] = "friendname";
    roles[ContentMsgRole] = "contentmsg";
    roles[PostDateRole] = "date";
    roles[VoteUpRole] = "voteup";
    roles[VoteDownRole] = "votedown";
    roles[CommentRole] = "qcomments";
    roles[MsgIdRole] = "msgid";
    roles[GrpIdRole] = "grpid";

    return roles;
}
