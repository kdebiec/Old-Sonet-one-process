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
#include "chatmsgmodel.h"

//libretroshare
#include <retroshare/rspeers.h>
#include <retroshare/rshistory.h>
#include <retroshare/rsidentity.h>

ChatMsgModel::ChatMsgModel(QObject *parent) : QAbstractListModel(parent)
{
    mChat = NULL;
    QObject::connect(NotifyTxt::getInstance(), SIGNAL(newMessage(bool)), this, SLOT(updateDisplay(bool)));
    QObject::connect(this,SIGNAL(gxsChanged()),this,SLOT(gxsSlot()));
    QObject::connect(this,SIGNAL(rspgpChanged()),this,SLOT(rspgpSlot()));
}

ChatMsgModel::~ChatMsgModel()
{
    if(mChat != NULL)
        delete mChat ;

    mChat = NULL ;
}

void ChatMsgModel::gxsSlot()
{
    setChatViaGxs(gxs);
}

void ChatMsgModel::rspgpSlot()
{
    setChatViaPgp(rspgp);
}

void ChatMsgModel::setChatViaGxs(QString gxs)
{
    RsGxsId gxsId(gxs.toStdString());
    RsIdentityDetails details;

    while(!rsIdentity->getIdDetails(gxsId, details))
        Sleep(500);

    entry = details.mPgpId;
    Chat *chat = new Chat;
    mChat = chat;
    mChat->setChatId(details.mPgpId);
    updateDisplay();
}

void ChatMsgModel::setChatViaPgp(QString pgp)
{
    entry = RsPgpId(pgp.toStdString());

    Chat *chat = new Chat;
    mChat = chat;
    mChat->setChatId(entry);

    updateDisplay();
}

void ChatMsgModel::updateDisplay(bool incoming)
{
    if(incoming)
        updateDisplay();
}

void ChatMsgModel::updateDisplay()
{
    beginResetModel();
    endResetModel();

    emit updatedDisplay();
}

int ChatMsgModel::rowCount(const QModelIndex & parent) const
{
    if(mChat != NULL)
    {
        int messageCount = 10000;
        std::list<HistoryMsg> historyMsgs;
        if(mChat->getChatId().toStdString().size() != 0)
        {
            while(!rsHistory->getMessages(mChat->getChatId(), historyMsgs, messageCount))
                Sleep(500);
        }

        return historyMsgs.size();
    }
    return 0;
}

QVariant ChatMsgModel::data(const QModelIndex & index, int role) const
{
    int idx = index.row();

    uint32_t hist_chat_type = 0xFFFF; // a value larger than the biggest RS_HISTORY_TYPE_* value
    int messageCount=0;

    if (mChat->getChatId().isLobbyId())
        hist_chat_type = RS_HISTORY_TYPE_LOBBY;
    else if (mChat->getChatId().isPeerId())
        hist_chat_type = RS_HISTORY_TYPE_PRIVATE ;
    else if (mChat->getChatId().isDistantChatId())
        hist_chat_type = RS_HISTORY_TYPE_PRIVATE ;
    else if(mChat->getChatId().isBroadcast())
        hist_chat_type = RS_HISTORY_TYPE_PUBLIC;

    if (rsHistory->getEnable(hist_chat_type))
    {
        // get chat messages from history
        std::list<HistoryMsg> historyMsgs;
        rsHistory->getMessages(mChat->getChatId(), historyMsgs, messageCount);

        std::list<HistoryMsg>::iterator historyIt;
        int i = 0;
        for (historyIt = historyMsgs.begin(); historyIt != historyMsgs.end(); ++historyIt)
        {
            if ( idx == i)
            {
                if (role == ContentMessageRole)
                    return QString::fromUtf8(historyIt->message.c_str());

                else if (role == NameRole)
                {
                    QString name;
                    if (mChat->getChatId().isLobbyId() || mChat->getChatId().isDistantChatId())
                    {
                        RsIdentityDetails details;
                        if (rsIdentity->getIdDetails(RsGxsId(historyIt->peerName), details))
                            name = QString::fromUtf8(details.mNickname.c_str());
                        else
                            name = QString::fromUtf8(historyIt->peerName.c_str());
                    }
                    else
                        name = QString::fromUtf8(historyIt->peerName.c_str());

                    return name;
                }
                else if (role == SendTimeRole)
                {
                    //return QDateTime::fromTime_t(historyIt->sendTime);
                }
                else if (role == RecvTimeRole)
                {
                    //return QDateTime::fromTime_t(historyIt->recvTime);
                }
                else if (role == SideRole)
                {
                    RsPeerId ownId = rsPeers->getOwnId();
                    QString ownName = QString::fromUtf8(rsPeers->getPeerName(ownId).c_str());

                    QString name;
                    if (mChat->getChatId().isLobbyId() || mChat->getChatId().isDistantChatId())
                    {
                        RsIdentityDetails details;
                        if (rsIdentity->getIdDetails(RsGxsId(historyIt->peerName), details))
                            name = QString::fromUtf8(details.mNickname.c_str());
                        else
                            name = QString::fromUtf8(historyIt->peerName.c_str());
                    }
                    else
                        name = QString::fromUtf8(historyIt->peerName.c_str());

                    if(ownName == name)
                        return 1;

                    return 2;
                }
            }
            ++i;
        }
    }
}

QHash<int, QByteArray> ChatMsgModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[ContentMessageRole] = "contentmessage";
    roles[NameRole] = "name";
    roles[SendTimeRole] = "sendtime";
    roles[RecvTimeRole] = "recvtime";
    roles[SideRole] = "side";

    return roles;
}
