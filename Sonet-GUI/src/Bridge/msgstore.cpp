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
#include "msgstore.h"

//libretroshare
#include <retroshare/rsmsgs.h>
#include <retroshare/rspeers.h>
#include <retroshare/rsidentity.h>

/*static*/ MsgStore *MsgStore::_instance = NULL;

/*static*/ MsgStore *MsgStore::Create()
{
    if (_instance == NULL)
        _instance = new MsgStore();

    return _instance;
}

/*static*/ void MsgStore::Destroy()
{
    if(_instance != NULL)
        delete _instance ;

    _instance = NULL ;
}

/*static*/ MsgStore *MsgStore::getInstance()
{
    return _instance;
}

MsgStore::MsgStore(QObject *parent): QObject(parent)
{
    storedMsg = new std::vector<MsgPreview>;
}

MsgStore::~MsgStore()
{
    if(storedMsg != NULL)
        delete storedMsg ;

    storedMsg = NULL ;
}

void MsgStore::storeMsg(MsgPreview &msg)
{
    if(msg.incoming)
    {
        storedMsg->push_back(msg);
        emit msgReceived();
    }
}

void MsgStore::storeMsgs(std::vector<MsgPreview> &msgs)
{
    storedMsg->insert(storedMsg->end(),msgs.begin(),msgs.end());
    emit msgReceived();
}

void MsgStore::cleanStoredMsgForPGP(QString rspgp)
{
    if(!storedMsg->empty())
    {
        RsPgpId entry(rspgp.toStdString());

        std::list<RsPeerId> sslContacts;
        rsPeers->getAssociatedSSLIds(entry, sslContacts);

        for (std::list<RsPeerId>::iterator sslIt = sslContacts.begin(); sslIt != sslContacts.end(); ++sslIt)
        {
            for (std::vector<MsgPreview>::iterator sMsg = storedMsg->begin(); sMsg != storedMsg->end();)
            {
                if(sMsg->peer == *sslIt)
                    sMsg = storedMsg->erase(sMsg);
                else
                    ++sMsg;
            }
        }
        emit msgStoreCleaned();
    }
}

std::vector<MsgPreview>* MsgStore::getStoredMsgs() const
{
    return storedMsg;
}
