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
#ifndef VNOTIFY_H
#define VNOTIFY_H

//Qt
#include <QObject>
#include <QMutex>

//libretroshare
#include <retroshare/rstypes.h>
#include <services/VOIP/rsnotifyvoip.h>

class VNotify : public QObject, public VOIPNotifyClient
{
    Q_OBJECT
public:

    static VNotify *Create ();
    static VNotify *getInstance ();
    void enable();

        virtual ~VNotify() { return; }

    virtual void notifyReceivedVoipAccept(const RsPeerId &peer_id, const uint32_t flags) ;
    virtual void notifyReceivedVoipBandwidth(const RsPeerId &peer_id, uint32_t bytes_per_sec) ;
    virtual void notifyReceivedVoipData(const RsPeerId &peer_id) ;
    virtual void notifyReceivedVoipHangUp(const RsPeerId &peer_id, const uint32_t flags) ;
    virtual void notifyReceivedVoipInvite(const RsPeerId &peer_id, const uint32_t flags) ;
    virtual void notifyReceivedVoipAudioCall(const RsPeerId &peer_id) ;
    virtual void notifyReceivedVoipVideoCall(const RsPeerId &peer_id) ;

signals:

    void voipAcceptReceived(const RsPeerId &peer_id, int flags) ; // emitted when the peer accepts the call
    void voipBandwidthInfoReceived(const RsPeerId &peer_id, int bytes_per_sec) ; // emitted when measured bandwidth info is received by the peer.
    void voipDataReceived(const RsPeerId &peer_id) ;			// signal emitted when some voip data has been received
    void voipHangUpReceived(const RsPeerId &peer_id, int flags) ; // emitted when the peer closes the call (i.e. hangs up)
    void voipInvitationReceived(const RsPeerId &peer_id, int flags) ;	// signal emitted when an invitation has been received
    void voipAudioCallReceived(const RsPeerId &peer_id) ; // emitted when the peer is calling and own don't send audio
    void voipVideoCallReceived(const RsPeerId &peer_id) ; // emitted when the peer is calling and own don't send video

private:
    VNotify() {}

    static VNotify *_instance;
    bool _enabled;

    QMutex _mutex;
};

#endif // VNOTIFY_H
