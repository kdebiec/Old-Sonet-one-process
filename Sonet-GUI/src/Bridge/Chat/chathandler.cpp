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
#include "chathandler.h"

//libretroshare
#include <retroshare/rspeers.h>
#include <retroshare/rsidentity.h>

//Sonet-GUI
#include "chat.h"

/*static*/ ChatHandler *ChatHandler::_instance = NULL;

/*static*/ ChatHandler *ChatHandler::Create()
{
    if (_instance == NULL)
        _instance = new ChatHandler();

    return _instance;
}

/*static*/ void ChatHandler::Destroy()
{
    if(_instance != NULL)
        delete _instance;

    _instance = NULL;
}

/*static*/ ChatHandler *ChatHandler::getInstance()
{
    return _instance;
}

void ChatHandler::sendMsgViaGxs(QString gxs, QString msg)
{
    RsGxsId gxsId(gxs.toStdString());
    RsIdentityDetails details;

    rsIdentity->getIdDetails(gxsId, details);

    RsPgpId entry;
    entry = details.mPgpId;

    Chat *chat = new Chat;
    chat->setChatId(entry);
    chat->sendMsg(msg);
    delete(chat);
}

void ChatHandler::sendMsgViaPGP(QString rs, QString msg)
{
    RsPgpId entry(rs.toStdString());

    Chat *chat = new Chat;
    chat->setChatId(entry);
    chat->sendMsg(msg);
    delete(chat);
}
