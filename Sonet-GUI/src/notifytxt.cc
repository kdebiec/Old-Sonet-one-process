/****************************************************************
 *  This file is part of Sonet.
 *  Sonet is distributed under the following license:
 *
 *  Copyright 2004-2006 by Robert Fernie.
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

#include "notifytxt.h"

//std
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sstream>

#ifdef WINDOWS_SYS
    #include <conio.h>
    #include <stdio.h>
#endif

//Qt
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
#include <QMutexLocker>

//libretroshare
#include <retroshare/rspeers.h>
#include <util/rsdir.h>

//Sonet-GUI
#include "Bridge/LoginWindow/loginwindow.h"
#include "Bridge/msgstore.h"

/*static*/ NotifyTxt *NotifyTxt::_instance = NULL;

/*static*/ NotifyTxt *NotifyTxt::Create ()
{
    if (_instance == NULL)
        _instance = new NotifyTxt ();

    return _instance;
}
/*static*/ NotifyTxt *NotifyTxt::getInstance ()
{
    return _instance;
}

NotifyTxt::NotifyTxt() : mNotifyMtx("NotifyMtx")
{
    {
        QMutexLocker m(&_mutex) ;
        _enabled = false ;
    }
}

class SignatureEventData
{
    public:
        SignatureEventData(const void *_data,int32_t _len,unsigned int _signlen, std::string _reason)
        {
            // We need a new memory chnk because there's no guarranty _sign nor _signlen are not in the stack

            sign = (unsigned char *)rs_malloc(_signlen);

            if(!sign)
            {
                signlen = NULL;
                signature_result = SELF_SIGNATURE_RESULT_FAILED;
                return;
            }

            signlen = new unsigned int;
            *signlen = _signlen;
            signature_result = SELF_SIGNATURE_RESULT_PENDING;
            data = rs_malloc(_len);

                    if(!data)
                    {
                        len = 0;
                        return;
                    }
            len = _len;
            memcpy(data,_data,len);
            reason = _reason;
        }

        ~SignatureEventData()
        {
            free(sign);
            delete signlen;
            free(data);
        }

        void performSignature()
        {
            if(rsPeers->gpgSignData(data,len,sign,signlen,reason))
                signature_result = SELF_SIGNATURE_RESULT_SUCCESS;
            else
                signature_result = SELF_SIGNATURE_RESULT_FAILED;
        }

        void *data;
        uint32_t len;
        unsigned char *sign;
        unsigned int *signlen;
        int signature_result;		// 0=pending, 1=done, 2=failed/cancelled.
        std::string reason;
};

bool NotifyTxt::askForDeferredSelfSignature(const void *data, const uint32_t len, unsigned char *sign, unsigned int *signlen,int& signature_result, std::string reason /*=""*/)
{
    {
        QMutexLocker m(&_mutex);

        std::cerr << "NotifyTxt:: deferred signature event requeted. " << std::endl;

        // Look into the queue

        Sha1CheckSum chksum = RsDirUtil::sha1sum((uint8_t*)data,len);

        std::map<std::string,SignatureEventData*>::iterator it = _deferred_signature_queue.find(chksum.toStdString());
        signature_result = SELF_SIGNATURE_RESULT_PENDING;

        if(it != _deferred_signature_queue.end())
        {
            signature_result = it->second->signature_result;

            if(it->second->signature_result != SELF_SIGNATURE_RESULT_PENDING)	// found it. Copy the result, and remove from the queue.
            {
                // We should check for the exact data match, for the sake of being totally secure.
                //
                std::cerr << "Found into queue: returning it" << std::endl;

                memcpy(sign,it->second->sign,*it->second->signlen);
                *signlen = *(it->second->signlen);

                delete it->second;
                _deferred_signature_queue.erase(it);
            }

            return true;		// already registered, but not done yet.
        }

        // Not found. Store in the queue and emit a signal.
        //
        std::cerr << "NotifyTxt:: deferred signature event requeted. Pushing into queue" << std::endl;

        SignatureEventData *edta = new SignatureEventData(data,len,*signlen, reason);

        _deferred_signature_queue[chksum.toStdString()] = edta;
    }
    emit deferredSignatureHandlingRequested();
    return true;
}

void NotifyTxt::notifyErrorMsg(int list, int type, std::string msg)
{
    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
            return ;
    }
    emit errorOccurred(list,type,QString::fromUtf8(msg.c_str())) ;
}


bool NotifyTxt::askForPluginConfirmation(const std::string& plugin_file_name, const std::string& plugin_file_hash)
{
    QMessageBox dialog;
    dialog.setWindowTitle(tr("Unregistered plugin/executable"));

    QString text ;
    text += tr( "RetroShare has detected an unregistered plugin. This happens in two cases:<UL><LI>Your RetroShare executable has changed.</LI><LI>The plugin has changed</LI></UL>Click on Yes to authorize this plugin, or No to deny it. You can change your mind later in Options -> Plugins, then restart." ) ;
    text += "<UL>" ;
    text += "<LI>Hash:\t" + QString::fromStdString(plugin_file_hash) + "</LI>" ;
    text += "<LI>File:\t" + QString::fromStdString(plugin_file_name) + "</LI>";
    text += "</UL>" ;

    dialog.setText(text) ;
    dialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No) ;

    int ret = dialog.exec();


    if (ret == QMessageBox::Yes)
        return true ;
    else
        return false;
}
void NotifyTxt::handleSignatureEvent()
{
    std::cerr << "NotifyTxt:: performing a deferred signature in the main GUI thread." << std::endl;

    static bool working = false ;

    if(!working)
    {
        working = true ;

        for(std::map<std::string,SignatureEventData*>::const_iterator it(_deferred_signature_queue.begin());it!=_deferred_signature_queue.end();++it)
            it->second->performSignature() ;

        working = false ;
    }
}

bool NotifyTxt::askForPassword(const std::string& title, const std::string& key_details, bool prev_is_bad, std::string& password,bool& cancelled)
{
    if(title == "")
    {
        if(!LoginWindow::getInstance()->getPass().empty())
        {
            cancelled = false;
            password = LoginWindow::getInstance()->getPass();
            return true;
        }
        else
        {
            QInputDialog dialog;
            dialog.setWindowTitle(tr("PGP key passphrase"));
            dialog.setLabelText((prev_is_bad ? QString("%1\n\n").arg(tr("Wrong password !")) : QString()) + QString("%1:\n    %2").arg(tr("Please enter your PGP password for key"), QString::fromUtf8(key_details.c_str())));
            dialog.setTextEchoMode(QLineEdit::Password);
            dialog.setModal(true);
            int ret = dialog.exec();

            cancelled = false;

            if (ret == QDialog::Rejected)
            {
                cancelled = true;
                password.clear();
                return true;
            }

            if (ret == QDialog::Accepted)
            {
                 password = dialog.textValue().toUtf8().constData();
                 return true;
            }
        }
    }
    else if (title == "AuthSSLimpl::SignX509ReqWithGPG()")
    {
        if(!LoginWindow::getInstance()->getPass().empty())
        {
            cancelled = false;
            password = LoginWindow::getInstance()->getPass();
            return true;
        }
        else
        {
            QInputDialog dialog;
            dialog.setWindowTitle(tr("You need to sign your node's certificate."));
            dialog.setLabelText((prev_is_bad ? QString("%1\n\n").arg(tr("Wrong password !")) : QString()) + QString("%1:\n    %2").arg(tr("Please enter your PGP password for key"), QString::fromUtf8(key_details.c_str())));
            dialog.setTextEchoMode(QLineEdit::Password);
            dialog.setModal(true);
            int ret = dialog.exec();

            cancelled = false;

            if (ret == QDialog::Rejected)
            {
                password.clear();
                cancelled = true;
                return true;
            }

            if (ret == QDialog::Accepted)
            {
                 password = dialog.textValue().toUtf8().constData();
                 return true;
            }
        }
    }
    else if (title == "p3IdService::service_CreateGroup()")
    {
        QInputDialog dialog;
        dialog.setWindowTitle(tr("You need to sign your forum/chatrooms identity."));
        dialog.setLabelText((prev_is_bad ? QString("%1\n\n").arg(tr("Wrong password !")) : QString()) + QString("%1:\n    %2").arg(tr("Please enter your PGP password for key"), QString::fromUtf8(key_details.c_str())));
        dialog.setTextEchoMode(QLineEdit::Password);
        dialog.setModal(true);
        int ret = dialog.exec();

        cancelled = false;

        if (ret == QDialog::Rejected)
        {
            password.clear();
            cancelled = true;
            emit passwordForIdentityRejected();
            return true;
        }

        if (ret == QDialog::Accepted)
        {
             password = dialog.textValue().toUtf8().constData();
             emit passwordForIdentityAccepted();
             return true;
        }
    }
    else
    {
        QInputDialog dialog;
        dialog.setWindowTitle(QString::fromStdString(title));
        dialog.setLabelText((prev_is_bad ? QString("%1\n\n").arg(tr("Wrong password !")) : QString()) + QString("%1:\n    %2").arg(tr("Please enter your PGP password for key"), QString::fromUtf8(key_details.c_str())));
        dialog.setTextEchoMode(QLineEdit::Password);
        dialog.setModal(true);
        int ret = dialog.exec();

        cancelled = false ;

        if (ret == QDialog::Rejected)
        {
            password.clear();
            cancelled = true;
            return true;
        }

        if (ret == QDialog::Accepted)
        {
             password = dialog.textValue().toUtf8().constData();
             return true;
        }
    }

    return false;
}

void NotifyTxt::notifyDiscInfoChanged()
{
    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
            return ;
    }

#ifdef NOTIFY_DEBUG
    std::cerr << "Notifyqt:: notified that discoveryInfo changed" << std::endl ;
#endif

    emit discInfoChanged() ;
}

void NotifyTxt::notifyDownloadComplete(const std::string& fileHash)
{
    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
            return ;
    }

#ifdef NOTIFY_DEBUG
    std::cerr << "Notifyqt::notifyDownloadComplete notified that a download is completed" << std::endl;
#endif

    emit downloadComplete(QString::fromStdString(fileHash));
}

void NotifyTxt::notifyDownloadCompleteCount(uint32_t count)
{
    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
            return ;
    }

    std::cerr << "Notifyqt::notifyDownloadCompleteCount " << count << std::endl;

    emit downloadCompleteCountChanged(count);
}

void NotifyTxt::notifyDiskFull(uint32_t loc,uint32_t size_in_mb)
{
    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
            return ;
    }

    std::cerr << "Notifyqt:: notified that disk is full" << std::endl ;

    emit diskFull(loc,size_in_mb) ;
}

/* peer has changed the state */
void NotifyTxt::notifyPeerStatusChanged(const std::string& peer_id, uint32_t state)
{
    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
            return ;
    }

#ifdef NOTIFY_DEBUG
    std::cerr << "Notifyqt:: notified that peer " << peer_id << " has changed the state to " << state << std::endl;
#endif

    emit peerStatusChanged(QString::fromStdString(peer_id), state);
}

/* one or more peers has changed the states */
void NotifyTxt::notifyPeerStatusChangedSummary()
{
    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
            return ;
    }

#ifdef NOTIFY_DEBUG
    std::cerr << "Notifyqt:: notified that one peer has changed the state" << std::endl;
#endif

    emit peerStatusChangedSummary();
}

void NotifyTxt::notifyGxsChange(const RsGxsChanges& changes)
{
    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
            return ;
    }

#ifdef NOTIFY_DEBUG
    std::cerr << "Notifyqt:: notified that gxs has changes" << std::endl;
#endif

    emit gxsChange(changes);
}

void NotifyTxt::notifyOwnStatusMessageChanged()
{
    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
            return ;
    }

#ifdef NOTIFY_DEBUG
    std::cerr << "Notifyqt:: notified that own avatar changed" << std::endl ;
#endif
    emit ownStatusMessageChanged() ;
}

void NotifyTxt::notifyPeerHasNewAvatar(std::string peer_id)
{
    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
            return ;
    }
#ifdef NOTIFY_DEBUG
    std::cerr << "notifyQt: notification of new avatar." << std::endl ;
#endif
    emit peerHasNewAvatar(QString::fromStdString(peer_id)) ;
}

void NotifyTxt::notifyCustomState(const std::string& peer_id, const std::string& status_string)
{
    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
            return ;
    }

#ifdef NOTIFY_DEBUG
    std::cerr << "notifyQt: Received custom status string notification" << std::endl ;
#endif
    emit peerHasNewCustomStateString(QString::fromStdString(peer_id), QString::fromUtf8(status_string.c_str())) ;
}

void NotifyTxt::notifyChatLobbyTimeShift(int shift)
{
    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
            return ;
    }

#ifdef NOTIFY_DEBUG
    std::cerr << "notifyQt: Received chat lobby time shift message: shift = " << shift << std::endl;
#endif
    emit chatLobbyTimeShift(shift) ;
}

void NotifyTxt::notifyChatLobbyEvent(uint64_t lobby_id,uint32_t event_type,const std::string& nickname,const std::string& str)
{
    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
            return ;
    }

#ifdef NOTIFY_DEBUG
    std::cerr << "notifyQt: Received chat lobby event message: lobby #" << std::hex << lobby_id  << std::dec << ", event=" << event_type << ", str=\"" << str << "\"" << std::endl ;
#endif
    emit chatLobbyEvent(lobby_id,event_type,QString::fromUtf8(nickname.c_str()),QString::fromUtf8(str.c_str())) ;
}

void NotifyTxt::notifyChatStatus(const ChatId& chat_id,const std::string& status_string)
{
    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
            return ;
    }

#ifdef NOTIFY_DEBUG
    std::cerr << "notifyQt: Received chat status string: " << status_string << std::endl ;
#endif
    emit chatStatusChanged(chat_id, QString::fromUtf8(status_string.c_str()));
}

void NotifyTxt::notifyHashingInfo(uint32_t type, const std::string& fileinfo)
{
    QString info;

    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
            return ;
    }

    switch (type)
    {
    case NOTIFY_HASHTYPE_EXAMINING_FILES:
        info = tr("Examining shared files...");
        break;
    case NOTIFY_HASHTYPE_FINISH:
        break;
    case NOTIFY_HASHTYPE_HASH_FILE:
        info = tr("Hashing file") + " " + QString::fromUtf8(fileinfo.c_str());
        break;
    case NOTIFY_HASHTYPE_SAVE_FILE_INDEX:
        info = tr("Saving file index...");
        break;
    }

    emit hashingInfoChanged(info);
}

void NotifyTxt::notifyHistoryChanged(uint32_t msgId, int type)
{
    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
            return ;
    }

    emit historyChanged(msgId, type);
}

void NotifyTxt::notifyListChange(int list, int type)
{
    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
            return ;
    }
#ifdef NOTIFY_DEBUG
    std::cerr << "NotifyTxt::notifyListChange()" << std::endl;
#endif
    switch(list)
    {
        case NOTIFY_LIST_NEIGHBOURS:
#ifdef NOTIFY_DEBUG
            std::cerr << "received neighbours changed" << std::endl ;
#endif
            emit neighboursChanged();
            break;
        case NOTIFY_LIST_FRIENDS:
#ifdef NOTIFY_DEBUG
            std::cerr << "received friends changed" << std::endl ;
#endif
            emit friendsChanged() ;
            break;
        case NOTIFY_LIST_DIRLIST_LOCAL:
#ifdef NOTIFY_DEBUG
            std::cerr << "received files changed" << std::endl ;
#endif
            emit filesPostModChanged(true) ;  /* Local */
            break;
        case NOTIFY_LIST_CHAT_LOBBY_INVITATION:
#ifdef NOTIFY_DEBUG
            std::cerr << "received files changed" << std::endl ;
#endif
            emit chatLobbyInviteReceived() ;  /* Local */
            break;
        case NOTIFY_LIST_DIRLIST_FRIENDS:
#ifdef NOTIFY_DEBUG
            std::cerr << "received files changed" << std::endl ;
#endif
            emit filesPostModChanged(false) ;  /* Local */
            break;
        case NOTIFY_LIST_SEARCHLIST:
            break;
        case NOTIFY_LIST_MESSAGELIST:
#ifdef NOTIFY_DEBUG
            std::cerr << "received msg changed" << std::endl ;
#endif
            emit messagesChanged() ;
            break;
        case NOTIFY_LIST_MESSAGE_TAGS:
#ifdef NOTIFY_DEBUG
            std::cerr << "received msg tags changed" << std::endl ;
#endif
            emit messagesTagsChanged();
            break;
        case NOTIFY_LIST_CHANNELLIST:
            break;
        case NOTIFY_LIST_TRANSFERLIST:
#ifdef NOTIFY_DEBUG
            std::cerr << "received transfer changed" << std::endl ;
#endif
            emit transfersChanged() ;
            break;
        case NOTIFY_LIST_CONFIG:
#ifdef NOTIFY_DEBUG
            std::cerr << "received config changed" << std::endl ;
#endif
            emit configChanged() ;
            break ;

#ifdef REMOVE
        case NOTIFY_LIST_FORUMLIST_LOCKED:
#ifdef NOTIFY_DEBUG
            std::cerr << "received forum msg changed" << std::endl ;
#endif
            emit forumsChanged(); // use connect with Qt::QueuedConnection
            break;
        case NOTIFY_LIST_CHANNELLIST_LOCKED:
#ifdef NOTIFY_DEBUG
            std::cerr << "received channel msg changed" << std::endl ;
#endif
            emit channelsChanged(type); // use connect with Qt::QueuedConnection
            break;
        case NOTIFY_LIST_PUBLIC_CHAT:
#ifdef NOTIFY_DEBUG
            std::cerr << "received public chat changed" << std::endl ;
#endif
            emit publicChatChanged(type);
            break;
        case NOTIFY_LIST_PRIVATE_INCOMING_CHAT:
        case NOTIFY_LIST_PRIVATE_OUTGOING_CHAT:
#ifdef NOTIFY_DEBUG
            std::cerr << "received private chat changed" << std::endl ;
#endif
            emit privateChatChanged(list, type);
            break;
#endif

        case NOTIFY_LIST_CHAT_LOBBY_LIST:
#ifdef NOTIFY_DEBUG
            std::cerr << "received notify chat lobby list" << std::endl;
#endif
            emit lobbyListChanged();
            break;

        case NOTIFY_LIST_GROUPLIST:
#ifdef NOTIFY_DEBUG
            std::cerr << "received groups changed" << std::endl ;
#endif
            emit groupsChanged(type);
            break;
        default:
            break;
    }
    return;
}

void NotifyTxt::notifyListPreChange(int list, int /*type*/)
{
    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
            return ;
    }

#ifdef NOTIFY_DEBUG
    std::cerr << "NotifyQt::notifyListPreChange()" << std::endl;
#endif
    switch(list)
    {
        case NOTIFY_LIST_NEIGHBOURS:
            break;
        case NOTIFY_LIST_FRIENDS:
            emit friendsChanged() ;
            break;
        case NOTIFY_LIST_DIRLIST_FRIENDS:
            emit filesPreModChanged(false) ;	/* remote */
            break ;
        case NOTIFY_LIST_DIRLIST_LOCAL:
            emit filesPreModChanged(true) ;	/* local */
            break;
        case NOTIFY_LIST_SEARCHLIST:
            break;
        case NOTIFY_LIST_MESSAGELIST:
            break;
        case NOTIFY_LIST_CHANNELLIST:
            break;
        case NOTIFY_LIST_TRANSFERLIST:
            break;
        default:
            break;
    }
    return;
}

void NotifyTxt::notifyOwnAvatarChanged()
{
    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
            return;
    }

#ifdef NOTIFY_DEBUG
    std::cerr << "Notifyqt:: notified that own avatar changed" << std::endl ;
#endif
    emit ownAvatarChanged() ;
}

void NotifyTxt::notifyChatMessage(const ChatMessage &msg)
{
    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
        {
            if(msg.incoming)
            {
                MsgPreview msgPreview;
                msgPreview.incoming = msg.incoming;
                msgPreview.peer = msg.chat_id.toPeerId();
                msgPreviewVector.push_back(msgPreview);
            }
            return;
        }
    }

#ifdef NOTIFY_DEBUG
    std::cerr << "notifyQt: Received chat message " << std::endl ;
#endif

    bool incoming = msg.incoming;

    if(msg.incoming)
    {
        MsgPreview msgPreview;
        msgPreview.incoming = msg.incoming;
        msgPreview.peer = msg.chat_id.toPeerId();

        if(MsgStore::getInstance() != NULL)
            MsgStore::getInstance()->storeMsg(msgPreview);
    }

    emit newMessage();
    emit newMessage(incoming);
}

void NotifyTxt::collectedMsg()
{
    if(MsgStore::getInstance() != NULL)
    {
        MsgStore::getInstance()->storeMsgs(msgPreviewVector);
        msgPreviewVector.clear();
    }
}

void NotifyTxt::enable()
{
    QMutexLocker m(&_mutex) ;
    std::cerr << "Enabling notification system" << std::endl;
    _enabled = true;
    collectedMsg();
}


/******************* Turtle Search Interface **********/

void NotifyTxt::notifyTurtleSearchResult(uint32_t search_id,const std::list<TurtleFileInfo>& files)
{
    {
        QMutexLocker m(&_mutex) ;
        if(!_enabled)
            return;
    }

#ifdef NOTIFY_DEBUG
    std::cerr << "in notify search result..." << std::endl ;
#endif

    for(std::list<TurtleFileInfo>::const_iterator it(files.begin());it!=files.end();++it)
    {
        FileDetail det ;
        det.rank = 0 ;
        det.age = 0 ;
        det.name = (*it).name ;
        det.hash = (*it).hash ;
        det.size = (*it).size ;
        det.id.clear() ;

        emit gotTurtleSearchResult(search_id,det) ;
    }
}


                /* interface for handling SearchResults */
void NotifyTxt::getSearchIds(std::list<uint32_t> &searchIds)
{
	RsStackMutex stack(mNotifyMtx); /****** LOCKED *****/

        std::map<uint32_t, std::list<TurtleFileInfo> >::iterator it;
	for(it = mSearchResults.begin(); it != mSearchResults.end(); it++)
        searchIds.push_back(it->first);

	return;
}


int NotifyTxt::getSearchResults(uint32_t id, std::list<TurtleFileInfo> &searchResults)
{
	RsStackMutex stack(mNotifyMtx); /****** LOCKED *****/

        std::map<uint32_t, std::list<TurtleFileInfo> >::iterator it;
	it = mSearchResults.find(id);
    if (it == mSearchResults.end())
        return 0;

	searchResults = it->second;
	return 1;
}


int NotifyTxt::getSearchResultCount(uint32_t id)
{
	RsStackMutex stack(mNotifyMtx); /****** LOCKED *****/

        std::map<uint32_t, std::list<TurtleFileInfo> >::iterator it;
	it = mSearchResults.find(id);
    if (it == mSearchResults.end())
        return 0;

	return it->second.size();
}

                // only collect results for selected searches.
                // will drop others.
int NotifyTxt::collectSearchResults(uint32_t searchId)
{
	std::cerr << "NotifyTxt::collectSearchResult(" << searchId << ")";
	std::cerr << std::endl;

	RsStackMutex stack(mNotifyMtx); /****** LOCKED *****/

        std::map<uint32_t, std::list<TurtleFileInfo> >::iterator it;
	it = mSearchResults.find(searchId);
	if (it == mSearchResults.end())
	{
        std::list<TurtleFileInfo> emptyList;
		mSearchResults[searchId] = emptyList;
		return 1;
	}

	std::cerr << "NotifyTxt::collectSearchResult() ERROR Id exists";
	std::cerr << std::endl;
	return 1;
}

int NotifyTxt::clearSearchId(uint32_t searchId)
{
	std::cerr << "NotifyTxt::clearSearchId(" << searchId << ")";
	std::cerr << std::endl;

	RsStackMutex stack(mNotifyMtx); /****** LOCKED *****/

        std::map<uint32_t, std::list<TurtleFileInfo> >::iterator it;
	it = mSearchResults.find(searchId);
	if (it == mSearchResults.end())
	{
		std::cerr << "NotifyTxt::clearSearchId() ERROR Id not there";
		std::cerr << std::endl;
		return 0;
	}

	mSearchResults.erase(it);
	return 1;
}


