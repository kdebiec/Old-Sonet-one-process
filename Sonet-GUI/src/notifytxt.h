#ifndef RSIFACE_NOTIFY_TXT_H
#define RSIFACE_NOTIFY_TXT_H
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

//std
#include <string>

//Qt
#include <QObject>
#include <QMutex>

//libretroshare
#include <retroshare/rsiface.h>
#include <retroshare/rsturtle.h>
#include <retroshare/rsnotify.h>
#include <retroshare/rsmsgs.h>
#include "util/rsthreads.h"

class SignatureEventData ;

struct MsgPreview{
    RsPeerId peer;
    bool incoming;
};

class NotifyTxt: public QObject, public NotifyClient
{
    Q_OBJECT
    public:
        static NotifyTxt *Create ();
        static NotifyTxt *getInstance ();
        static bool isAllDisable();
        void enable();

		virtual ~NotifyTxt() { return; }


        virtual void notifyListPreChange            (int list, int type);
        virtual void notifyListChange               (int list, int type);
        virtual void notifyErrorMsg                 (int list, int sev, std::string msg);
        virtual void notifyChatMessage              (const ChatMessage&        /* msg */);
        virtual void notifyChatStatus               (const ChatId &chat_id,const std::string& status_string);
        virtual void notifyChatLobbyEvent           (uint64_t /* lobby id */,uint32_t /* event type */,const std::string& /*nickname*/,const std::string& /* any string */) ;
        virtual void notifyChatLobbyTimeShift       (int time_shift) ;
        virtual void notifyCustomState              (const std::string& peer_id, const std::string& status_string);
        virtual void notifyHashingInfo              (uint32_t type, const std::string& fileinfo);
        virtual void notifyTurtleSearchResult       (uint32_t search_id,const std::list<TurtleFileInfo>& found_files);
        virtual void notifyPeerHasNewAvatar         (std::string peer_id) ;
        virtual void notifyOwnAvatarChanged         () ;
        virtual void notifyOwnStatusMessageChanged  () ;
        virtual void notifyDiskFull                 (uint32_t loc,uint32_t size_in_mb) ;
        /* peer has changed the state */
        virtual void notifyPeerStatusChanged        (const std::string& peer_id, uint32_t state);
        virtual void notifyGxsChange                (const RsGxsChanges& change);

        /* one or more peers has changed the states */
        virtual void notifyPeerStatusChangedSummary ();

        virtual void notifyDiscInfoChanged          () ;

        virtual void notifyDownloadComplete         (const std::string& fileHash);
        virtual void notifyDownloadCompleteCount    (uint32_t count);
        virtual void notifyHistoryChanged           (uint32_t msgId, int type);

        virtual bool askForPassword                 (const std::string& title, const std::string& key_details, bool prev_is_bad, std::string& password, bool& cancelled);
        virtual bool askForPluginConfirmation       (const std::string& plugin_file, const std::string& plugin_hash);

		/* interface for handling SearchResults */
		void getSearchIds(std::list<uint32_t> &searchIds);

		int getSearchResultCount(uint32_t id);
		int getSearchResults(uint32_t id, std::list<TurtleFileInfo> &searchResults);

		// only collect results for selected searches.
		// will drop others.
		int collectSearchResults(uint32_t searchId);
		int clearSearchId(uint32_t searchId);

        /* Just for notifying about collected msg after enabling notifying  */

        void collectedMsg();

        // Queues the signature event so that it canhappen in the main GUI thread (to ask for passwd).
        // To use this function: call is multiple times as soon as it returns true.
        //
        // Dont' use a while, if you're in a mutexed part, otherwize it will lock. You need to call the function
        // and periodically exit the locked code between calls to allow the signature to happen.
        //
        // Returns:
        // 	false = the signature is registered, but the result is not there yet. Call again soon.
        // 	true  = signature done. Data is ready. signature_result takes the following values:
        // 					1: signature success
        // 					2: signature failed. Wrong passwd, user pressed cancel, etc.
        //
        virtual bool askForDeferredSelfSignature(const void *data, const uint32_t len, unsigned char *sign, unsigned int *signlen,int& signature_result, std::string reason = "") ;

    signals:

        void hashingInfoChanged(const QString&) const ;
        void filesPreModChanged(bool) const ;
        void filesPostModChanged(bool) const ;
        void transfersChanged() const ;
        void friendsChanged() const ;
        void lobbyListChanged() const ;
        void chatLobbyEvent(qulonglong,int,const QString&,const QString&) ;
        void neighboursChanged() const ;
        void messagesChanged() const ;
        void messagesTagsChanged() const;

        void configChanged() const ;
        void chatStatusChanged(const ChatId&,const QString&) const ;
        void peerHasNewCustomStateString(const QString& /* peer_id */, const QString& /* status_string */) const ;
        void gotTurtleSearchResult(qulonglong search_id,FileDetail file) const ;
        void peerHasNewAvatar(const QString& peer_id) const ;
        void ownAvatarChanged() const ;
        void ownStatusMessageChanged() const ;
        void errorOccurred(int,int,const QString&) const ;
        void diskFull(int,int) const ;
        void peerStatusChanged(const QString& /* peer_id */, int /* status */);
        void peerStatusChangedSummary() const;
        void gxsChange(const RsGxsChanges& /* changes  */);

        //////////
        void newMessage();
        void newMessage(bool incoming);
        /////////
        void groupsChanged(int type) const ;
        void discInfoChanged() const ;
        void downloadComplete(const QString& /* fileHash */);
        void downloadCompleteCountChanged(int /* count */);

        void historyChanged(uint msgId, int type);
        void chatLobbyInviteReceived() ;
        void deferredSignatureHandlingRequested() ;
        void chatLobbyTimeShift(int time_shift) ;

        void passwordForIdentityRejected();
        void passwordForIdentityAccepted();

private slots:
    void handleSignatureEvent() ;

	private:

        NotifyTxt();

        static NotifyTxt *_instance;

        bool _enabled;
        QMutex _mutex;

        std::vector<MsgPreview> msgPreviewVector;

		/* store TurtleSearchResults */
		RsMutex mNotifyMtx;

		std::map<uint32_t, std::list<TurtleFileInfo> > mSearchResults;

        std::map<std::string, SignatureEventData*> _deferred_signature_queue ;
};

#endif
