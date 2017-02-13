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
#include "pgplistmodel.h"

//Qt
#include <QDateTime>
#include <QPainter>
#include <QTimer>

//libretroshare
#include <rsserver/rsaccounts.h>
#include <retroshare/rspeers.h>
#include <retroshare/rsstatus.h>
#include <retroshare/rsmsgs.h>

//Sonet-GUI
#include "msgstore.h"
#include "notifytxt.h"

// states for sorting (equal values are possible)
// used in BuildSortString - state + name
#define PEER_STATE_ONLINE       1
#define PEER_STATE_BUSY         2
#define PEER_STATE_AWAY         3
#define PEER_STATE_AVAILABLE    4
#define PEER_STATE_INACTIVE     5
#define PEER_STATE_OFFLINE      6

/*static*/ PGPListModel *PGPListModel::_instance = NULL;

/*static*/ PGPListModel *PGPListModel::Create()
{
    if (_instance == NULL)
        _instance = new PGPListModel();

    return _instance;
}

/*static*/ void PGPListModel::Destroy()
{
    if(_instance != NULL)
        delete _instance ;

    _instance = NULL ;
}

/*static*/ PGPListModel *PGPListModel::getInstance()
{
    return _instance;
}

PGPListModel::PGPListModel(QObject *parent) : QAbstractListModel(parent)
{
    requestedUpdate = false;

    _timer = new QTimer;
    _timer->setInterval(3000);
    _timer->setSingleShot(true);

    QObject::connect(_timer,SIGNAL(timeout()),this,SLOT(loadRequest()));

    _timer->start();

    QObject::connect(NotifyTxt::getInstance(), SIGNAL(friendsChanged()), this, SLOT(updateDisplaySize()));
    QObject::connect(NotifyTxt::getInstance(), SIGNAL(peerStatusChanged(QString, int)), this, SLOT(requestUpdate(QString, int)));
    QObject::connect(NotifyTxt::getInstance(), SIGNAL(peerHasNewCustomStateString(QString, QString)), this, SLOT(requestUpdate(QString, QString)));

    QObject::connect(MsgStore::getInstance(), SIGNAL(msgReceived()), this, SLOT(requestUpdate()));
    QObject::connect(MsgStore::getInstance(), SIGNAL(msgStoreCleaned()), this, SLOT(requestUpdate()));
}

PGPListModel::~PGPListModel()
{
    if(_timer != NULL)
        delete _timer ;

    _timer = NULL ;
}

void PGPListModel::requestUpdate()
{
    requestedUpdate = true;
}

void PGPListModel::requestUpdate(QString, int)
{
    requestedUpdate = true;
}

void PGPListModel::requestUpdate(QString, QString)
{
    requestedUpdate = true;
}

void PGPListModel::loadRequest()
{
    if(requestedUpdate)
    {
        updateDisplayData();
        requestedUpdate = false;
    }
    _timer->start();
}

void PGPListModel::updateDisplaySize()
{
#ifdef PGPLISTMODEL_DEBUG
    std::cerr << "updateDisplaySize()" << std::endl;
#endif
    beginResetModel();
    endResetModel();
}

void PGPListModel::updateDisplayData()
{
#ifdef PGPLISTMODEL_DEBUG
    std::cerr << "updateDisplayData()" << std::endl;
#endif

    for (int idx = 0; idx < rowCount(); idx++)
    {
        emit dataChanged(index(idx),index(idx));

#ifdef PGPLISTMODEL_DEBUG
        std::cerr << "emitted" << std::endl;
#endif
    }
}

int PGPListModel::rowCount(const QModelIndex & parent) const
{
    std::list<RsPgpId> gpg;
    rsPeers->getGPGAcceptedList(gpg);
    return gpg.size();
}

QVariant PGPListModel::data(const QModelIndex & index, int role) const
{
    std::list<StatusInfo> statusInfo;
    rsStatus->getStatusList(statusInfo);

    std::list<RsPgpId> gpg;
    rsPeers->getGPGAcceptedList(gpg);

#ifdef PGPLISTMODEL_DEBUG
    std::cerr << "QVariant PGPListModel::data(const QModelIndex & index, int role) const start job" << std::endl;
#endif

    int idx = index.row();

    if (role == Index)
        return idx;

    if (idx < 0 || idx >= gpg.size())
        return QVariant("Something goes wrong... :(");

    RsPgpId entry;

    std::list<RsPgpId> gpgF = gpg;
    std::list<RsPgpId>::iterator gpgIte;
    int i = 0;
    for (gpgIte = gpgF.begin(); gpgIte != gpgF.end(); ++gpgIte)
    {
        if ( idx == i)
        {
            entry = *gpgIte;

#ifdef PGPLISTMODEL_DEBUG
            std::cerr << "PGPListModel::data finded gpgFriends" << std::endl;
#endif
            break;
        }
      ++i;
    }

    RsPeerDetails detail;
    rsPeers->getGPGDetails(entry, detail);

    if (role == FriendNameRole)
        return QString::fromStdString(detail.name);

    if (role == RsPgpIdRole)
        return QString::fromStdString(entry.toStdString());

    int bestPeerState = 0;        // for gpg item
    unsigned int bestRSState = 0; // for gpg item
    QString bestCustomStateString;// for gpg item
    std::list<RsPeerId> sslContacts;

    int msgCount = 0;

    rsPeers->getAssociatedSSLIds(detail.gpg_id, sslContacts);
    for (std::list<RsPeerId>::iterator sslIt = sslContacts.begin(); sslIt != sslContacts.end(); ++sslIt)
    {
        RsPeerId sslId = *sslIt;
        RsPeerDetails sslDetail;

        if (!rsPeers->getPeerDetails(sslId, sslDetail) || !rsPeers->isFriend(sslId))
            continue;

        /* Custom state string */
        QString customStateString;
        if (sslDetail.state & RS_PEER_STATE_CONNECTED)
        {
            customStateString = QString::fromUtf8(rsMsgs->getCustomStateString(sslDetail.id).c_str());
        }

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
                        bestCustomStateString = customStateString;
                    }
                    else if (peerState == bestPeerState)
                    {
                        /* equal state */
                        if (bestCustomStateString.isEmpty() && !customStateString.isEmpty())
                        {
                            bestPeerState = peerState;
                            bestRSState = rsState;
                            bestCustomStateString = customStateString;
                        }
                    }
                    break;
                }
            }
        }

        for (std::vector<MsgPreview>::const_iterator sMsg = MsgStore::getInstance()->getStoredMsgs()->begin(); sMsg != MsgStore::getInstance()->getStoredMsgs()->end(); ++sMsg)
        {
            if(sMsg->peer == sslId && sMsg->incoming == true)
                msgCount++;
        }
    }

    if(role == StatusMsgRole)
        return bestCustomStateString;

    if (role == StatusRole)
    {
        if(bestRSState == RS_STATUS_ONLINE)
            return QColor("#4caf50"); //green
        else if(bestRSState == RS_STATUS_BUSY)
            return QColor("#FF5722"); //red
        else if(bestRSState == RS_STATUS_AWAY)
            return QColor("#FFEB3B"); //yellow
        else
            return QColor("#9E9E9E");
    }

    if(MsgCountRole)
        return msgCount;
}

QHash<int, QByteArray> PGPListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[FriendNameRole] = "pgpname";
    roles[StatusRole] = "statuscolor";
    roles[Index] = "indexofdelegate";
    roles[RsPgpIdRole] = "rspgpid";
    roles[StatusMsgRole] = "statusmsg";
    roles[MsgCountRole] = "msgcount";

    return roles;
}
