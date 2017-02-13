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
#include "gxsidmodel.h"

//libretroshare
#include <retroshare/rspeers.h>
#include <retroshare/rsstatus.h>
#include <retroshare/rsmsgs.h>

//Sonet-GUI
#include "msgstore.h"
#include "notifytxt.h"

// Data Requests.
#define IDDIALOG_IDLIST     1
#define IDDIALOG_IDDETAILS  2
#define IDDIALOG_REPLIST    3
#define IDDIALOG_REFRESH    4

// states for sorting (equal values are possible)
// used in BuildSortString - state + name
#define PEER_STATE_ONLINE       1
#define PEER_STATE_BUSY         2
#define PEER_STATE_AWAY         3
#define PEER_STATE_AVAILABLE    4
#define PEER_STATE_INACTIVE     5
#define PEER_STATE_OFFLINE      6

/// //////////////////////////
/// \brief getHumanReadableDuration
/// \param seconds
/// \return QString of time
/// Function getted from retroshare-gui IdDialog.cpp
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

/*static*/ GxsIdModel *GxsIdModel::_instance = NULL;

/*static*/ GxsIdModel *GxsIdModel::Create()
{
    if (_instance == NULL)
        _instance = new GxsIdModel();

    return _instance;
}

/*static*/ void GxsIdModel::Destroy()
{
    if(_instance != NULL)
        delete _instance ;

    _instance = NULL ;
}

/*static*/ GxsIdModel *GxsIdModel::getInstance()
{
    return _instance;
}

GxsIdModel::GxsIdModel(QObject *parent)
:QAbstractListModel(parent)
{
    mIdQueue = new TokenQueue(rsIdentity->getTokenService(), this);
    checked = false;
    requestedUpdate = false;
    updateData();

    _timer = new QTimer;
    _timer->setInterval(60000);
    _timer->setSingleShot(true);

    QObject::connect(_timer,SIGNAL(timeout()),this,SLOT(updateDisplaySize())) ;

    _timer->start() ;

    _timer2 = new QTimer;
    _timer2->setInterval(500);
    _timer2->setSingleShot(true);

    QObject::connect(_timer2,SIGNAL(timeout()),this,SLOT(loadRequest()));

    _timer2->start();


    QObject::connect(NotifyTxt::getInstance(), SIGNAL(friendsChanged()), this, SLOT(updateDisplaySize()));
    QObject::connect(NotifyTxt::getInstance(), SIGNAL(peerStatusChanged(QString, int)), this, SLOT(requestUpdate(QString, int)));
    QObject::connect(NotifyTxt::getInstance(), SIGNAL(peerHasNewCustomStateString(QString, QString)), this, SLOT(requestUpdate(QString, QString)));

    QObject::connect(MsgStore::getInstance(), SIGNAL(msgReceived()), this, SLOT(requestUpdate()));
    QObject::connect(MsgStore::getInstance(), SIGNAL(msgStoreCleaned()), this, SLOT(requestUpdate()));
}

GxsIdModel::~GxsIdModel()
{
    delete(mIdQueue);
    if(_timer != NULL)
        delete _timer;

    _timer = NULL;

    if(_timer2 != NULL)
        delete _timer2;

    _timer2 = NULL;
}

void GxsIdModel::updateDisplaySize()
{
#ifdef GXSIDMODEL_DEBUG
    std::cerr << "GxsIdModel::updateDisplaySize()" << std::endl;
#endif

    updateData();
    beginResetModel();
    endResetModel();

    _timer->start() ;
}

void GxsIdModel::requestUpdate()
{
    requestedUpdate = true;
}

void GxsIdModel::requestUpdate(QString, int)
{
    requestedUpdate = true;
}

void GxsIdModel::requestUpdate(QString, QString)
{
    requestedUpdate = true;
}

void GxsIdModel::loadRequest()
{
    if(requestedUpdate)
    {
        updateDisplay();
        requestedUpdate = false;
    }
    _timer2->start();
}

void GxsIdModel::updateDisplay()
{
    updateData();
    for (int idx = 0; idx < rowCount(); idx++)
        emit dataChanged(index(idx),index(idx));
}

void GxsIdModel::updateData()
{
    if (!mIdQueue)
        return;

    RsTokReqOptions opts;
    opts.mReqType = GXS_REQUEST_TYPE_GROUP_DATA;

    uint32_t token;

    mIdQueue->requestGroupInfo(token, RS_TOKREQ_ANSTYPE_DATA, opts, IDDIALOG_IDLIST);

    while(!mIdQueue->checkForRequest(token))
        Sleep(500);

    datavector.clear();

    if (!rsIdentity->getGroupData(token, datavector))
    {
        std::cerr << "GxsIdModel::updateData() Error getting GroupData" << std::endl;
        return;
    }

    if(!datavector.empty())
    {
        for(std::vector<RsGxsIdGroup>::iterator gidIt = datavector.begin(); gidIt != datavector.end();)
        {
            if(!rsPeers->isGPGAccepted(gidIt->mPgpId))
                gidIt = datavector.erase(gidIt);
            else
                ++gidIt;
        }
        if(!checked)
        {
            for(std::vector<RsGxsIdGroup>::iterator gidIt = datavector.begin(); gidIt != datavector.end(); ++gidIt)
            {
                if(!rsIdentity->isARegularContact(gidIt->mMeta.mAuthorId))
                    rsIdentity->setAsRegularContact(gidIt->mMeta.mAuthorId, true);
            }
            checked = true;
        }
    }
}

int GxsIdModel::rowCount(const QModelIndex & parent) const
{
    return datavector.size();
}

QVariant GxsIdModel::data(const QModelIndex & index, int role) const
{
    int idx = index.row();

    if (idx < 0 || idx >= datavector.size())
        return QVariant("Something goes wrong... :(");

    if (role == Index)
        return idx;

    RsGxsIdGroup entry;

    int i = 0;
    for (std::vector<RsGxsIdGroup>::const_iterator vit = datavector.begin(); vit != datavector.end(); ++vit)
    {
        if ( idx == i)
        {
            entry = *vit;
            break;
        }
      ++i;
    }

    if (role == FriendNameRole)
        return QString::fromUtf8(entry.mMeta.mGroupName.c_str());

    if (role == StatusColorRole)
    {
        if (entry.mPgpKnown)
        {
            RsPeerDetails details;
            rsPeers->getGPGDetails(entry.mPgpId, details);

            std::list<StatusInfo> statusInfo;
            rsStatus->getStatusList(statusInfo);

            unsigned int bestRSState = 0;
            int bestPeerState = 0;
            std::list<RsPeerId> sslContacts;

            rsPeers->getAssociatedSSLIds(entry.mPgpId, sslContacts);
            for (std::list<RsPeerId>::iterator sslIt = sslContacts.begin(); sslIt != sslContacts.end(); ++sslIt)
            {
                RsPeerId sslId = *sslIt;
                RsPeerDetails sslDetail;

                if (!rsPeers->getPeerDetails(sslId, sslDetail) || !rsPeers->isFriend(sslId))
                    continue;

                int peerState = 0;
                if (sslDetail.state & RS_PEER_STATE_CONNECTED)
                {
                    // get the status info for this ssl id
                    int rsState = 0;
                    std::list<StatusInfo>::iterator it;
                    for (it = statusInfo.begin(); it != statusInfo.end(); ++it)
                    {
                        if (it->id == sslId)
                        {
                            rsState = it->status;
                            switch (rsState)
                            {
                            case RS_STATUS_INACTIVE:
                                peerState = PEER_STATE_INACTIVE;
                                break;

                            case RS_STATUS_ONLINE:
                                peerState = PEER_STATE_ONLINE;
                                break;

                            case RS_STATUS_AWAY:
                                peerState = PEER_STATE_AWAY;
                                break;

                            case RS_STATUS_BUSY:
                                peerState = PEER_STATE_BUSY;
                                break;
                            }

                            /* find the best ssl contact for the gpg item */
                            if (bestPeerState == 0 || peerState < bestPeerState)
                            {
                                bestPeerState = peerState;
                                bestRSState = rsState;
                            }
                            else if (peerState == bestPeerState)
                            {
                                bestPeerState = peerState;
                                bestRSState = rsState;
                            }
                        }
                    }
                }

            }
            if(bestRSState == RS_STATUS_ONLINE)
                return QColor("#4caf50"); //green
            else if(bestRSState == RS_STATUS_BUSY)
                return QColor("#FF5722"); //red
            else if(bestRSState == RS_STATUS_AWAY)
                return QColor("#FFEB3B"); //yellow
            else
                return QColor("#9E9E9E");
        }
        return QVariant();
    }

    if (role == GxsIdRole)
        return QString::fromStdString(entry.mMeta.mGroupId.toStdString());

    if (role == LastSeenRole)
    {
        time_t now = time(NULL);
        return getHumanReadableDuration(now - entry.mLastUsageTS);
    }

    if (role == StatusMsgRole)
    {
        if (entry.mPgpKnown)
        {
            std::list<StatusInfo> statusInfo;
            rsStatus->getStatusList(statusInfo);

            QString bestCustomStateString;// for gpg item
            std::list<RsPeerId> sslContacts;

            rsPeers->getAssociatedSSLIds(entry.mPgpId, sslContacts);
            for (std::list<RsPeerId>::iterator sslIt = sslContacts.begin(); sslIt != sslContacts.end(); ++sslIt)
            {
                RsPeerId sslId = *sslIt;
                RsPeerDetails sslDetail;

                if (!rsPeers->getPeerDetails(sslId, sslDetail) || !rsPeers->isFriend(sslId))
                    continue;

                /* Custom state string */
                QString customStateString;
                if (sslDetail.state & RS_PEER_STATE_CONNECTED)
                    customStateString = QString::fromUtf8(rsMsgs->getCustomStateString(sslDetail.id).c_str());

                if(!customStateString.isEmpty())
                    bestCustomStateString = customStateString;
            }
            return bestCustomStateString;
        }
        return QString("");
    }
    if(role == MsgCountRole)
    {
        int msgCount = 0;
        if(entry.mPgpKnown)
        {
            RsPeerDetails details;
            rsPeers->getGPGDetails(entry.mPgpId, details);

            std::list<RsPeerId> sslContacts;

            rsPeers->getAssociatedSSLIds(entry.mPgpId, sslContacts);
            for (std::list<RsPeerId>::iterator sslIt = sslContacts.begin(); sslIt != sslContacts.end(); ++sslIt)
            {
                for (std::vector<MsgPreview>::const_iterator sMsg = MsgStore::getInstance()->getStoredMsgs()->begin(); sMsg != MsgStore::getInstance()->getStoredMsgs()->end(); ++sMsg)
                {
                    if(sMsg->peer == *sslIt && sMsg->incoming == true)
                        msgCount++;
                }
            }
        }
        return msgCount;
    }
    if(role == RsPgpIdRole)
    {
        if(entry.mPgpKnown)
            return QString::fromStdString(entry.mPgpId.toStdString());
    }
}

QHash<int, QByteArray> GxsIdModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[FriendNameRole] = "friendname";
    roles[StatusColorRole] = "statuscolor";
    roles[Index] = "indexofdelegate";
    roles[GxsIdRole] = "gxsid";
    roles[LastSeenRole] = "lastseen";
    roles[StatusMsgRole] = "statusmsg";
    roles[MsgCountRole] = "msgcount";
    roles[RsPgpIdRole] = "rspgpid";

    return roles;
}
