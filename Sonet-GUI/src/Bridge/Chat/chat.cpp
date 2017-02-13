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
#include "chat.h"

//libretroshare
#include <retroshare/rspeers.h>

void Chat::sendMsg(QString& msg)
{
    if (msg.isEmpty())
        return;

    rsMsgs->sendChat(mChatId, msg.toUtf8().constData());
}

void Chat::setChatId(const RsPgpId &gpgId)
{
    if (gpgId.isNull())
        return;

    RsPeerDetails detail;
    if (!rsPeers->getGPGDetails(gpgId, detail))
        return;

    if (!detail.isOnlyGPGdetail)
        return;

    //let's get the ssl child details
    std::list<RsPeerId> sslIds;
    rsPeers->getAssociatedSSLIds(detail.gpg_id, sslIds);

    if (sslIds.size() == 1)
    {
        // chat with the one ssl id (online or offline)
        setChatId(ChatId(sslIds.front()));
        return;
    }

    // more than one ssl ids available, check for online
    std::list<RsPeerId> onlineIds;
    for (std::list<RsPeerId>::iterator it = sslIds.begin(); it != sslIds.end(); ++it)
    {
        if (rsPeers->isOnline(*it))
            onlineIds.push_back(*it);
    }

    if (onlineIds.size() == 1)
    {
        // chat with the online ssl id
        setChatId(ChatId(onlineIds.front()));
        return;
    }
}

void Chat::setChatId(const ChatId &peerId)
{
    this->mChatId = peerId;
}
