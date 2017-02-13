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
#include "statuscolor.h"

//libretroshare
#include <retroshare/rspeers.h>
#include <retroshare/rsstatus.h>

// states for sorting (equal values are possible)
// used in BuildSortString - state + name
#define PEER_STATE_ONLINE       1
#define PEER_STATE_BUSY         2
#define PEER_STATE_AWAY         3
#define PEER_STATE_AVAILABLE    4
#define PEER_STATE_INACTIVE     5
#define PEER_STATE_OFFLINE      6

StatusColor::StatusColor(QObject *parent) : QObject(parent)
{
    QObject::connect(NotifyTxt::getInstance(), SIGNAL(peerStatusChanged(QString, int)), this, SLOT(updateStatus()));
    QObject::connect(this,SIGNAL(rspgpChanged()),this,SLOT(rspgpSlot()));
}

void StatusColor::rspgpSlot()
{
    entry = RsPgpId(rspgp.toStdString());
    updateStatus();
}

void StatusColor::updateStatus()
{
    if(!entry.isNull())
    {
        std::list<StatusInfo> statusInfo;
        rsStatus->getStatusList(statusInfo);

        RsPeerDetails detail;
        rsPeers->getGPGDetails(entry, detail);

        int bestPeerState;        // for gpg item
        unsigned int bestRSState; // for gpg item
        std::list<RsPeerId> sslContacts;

        rsPeers->getAssociatedSSLIds(detail.gpg_id, sslContacts);
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
                                bestRSState = rsState;
                        break;
                    }
                }
            }
        }

        if(bestRSState == RS_STATUS_ONLINE)
        {
            color = QColor("#4caf50"); //green
        }
        else if(bestRSState == RS_STATUS_BUSY)
        {
            color = QColor("#FF5722"); //red
        }
        else if(bestRSState == RS_STATUS_AWAY)
        {
            color = QColor("#FFEB3B"); //yellow
        }
        else
        {
            color = QColor("#9E9E9E");
        }

        emit colorChanged();
    }
}
