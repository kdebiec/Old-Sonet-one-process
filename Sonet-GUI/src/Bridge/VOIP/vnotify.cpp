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
#include "vnotify.h"

/*static*/ VNotify *VNotify::_instance = NULL;

/*static*/ VNotify *VNotify::Create ()
{
    if (_instance == NULL)
        _instance = new VNotify();

    return _instance;
}
/*static*/ VNotify *VNotify::getInstance ()
{
    return _instance;
}

void VNotify::enable()
{
    QMutexLocker m(&_mutex);
#ifdef VNOTIFY_DEBUG
    std::cerr << "Enabling VOIP notification system" << std::endl;
#endif
    _enabled = true;
}

void VNotify::notifyReceivedVoipAccept(const RsPeerId& peer_id, const uint32_t flags)
{
    {
        QMutexLocker m(&_mutex);
        if(!_enabled)
            return;
    }

#ifdef VNOTIFY_DEBUG
    std::cerr << "VOIPNotify::notifyReceivedVoipAccept" << std::endl;
#endif

    emit voipAcceptReceived(peer_id, flags);
}
void VNotify::notifyReceivedVoipBandwidth(const RsPeerId &peer_id,uint32_t bytes_per_sec)
{
    {
        QMutexLocker m(&_mutex);
        if(!_enabled)
            return;
    }

#ifdef VNOTIFY_DEBUG
    std::cerr << "VOIPNotify::notifyReceivedVoipBandwidth" << std::endl;
#endif

    emit voipBandwidthInfoReceived(peer_id, bytes_per_sec);
}
void VNotify::notifyReceivedVoipData(const RsPeerId &peer_id)
{
    {
        QMutexLocker m(&_mutex);
        if(!_enabled)
            return;
    }

#ifdef VNOTIFY_DEBUG
    std::cerr << "VOIPNotify::notifyReceivedVoipData" << std::endl;
#endif

    emit voipDataReceived(peer_id);
}
void VNotify::notifyReceivedVoipHangUp(const RsPeerId &peer_id, const uint32_t flags)
{
    {
        QMutexLocker m(&_mutex);
        if(!_enabled)
            return;
    }

#ifdef VNOTIFY_DEBUG
    std::cerr << "VOIPNotify::notifyReceivedVoipHangUp" << std::endl;
#endif

    emit voipHangUpReceived(peer_id, flags);
}
void VNotify::notifyReceivedVoipInvite(const RsPeerId& peer_id, const uint32_t flags)
{
    {
        QMutexLocker m(&_mutex);
        if(!_enabled)
            return;
    }

#ifdef VNOTIFY_DEBUG
    std::cerr << "VOIPNotify::notifyReceivedVoipInvite" << std::endl;
#endif

    emit voipInvitationReceived(peer_id, flags);
}
void VNotify::notifyReceivedVoipAudioCall(const RsPeerId &peer_id)
{
    {
        QMutexLocker m(&_mutex);
        if(!_enabled)
            return;
    }

#ifdef VNOTIFY_DEBUG
    std::cerr << "VOIPNotify::notifyReceivedVoipAudioCall" << std::endl;
#endif

    emit voipAudioCallReceived(peer_id);
}
void VNotify::notifyReceivedVoipVideoCall(const RsPeerId &peer_id)
{
    {
        QMutexLocker m(&_mutex);
        if(!_enabled)
            return;
    }

#ifdef VNOTIFY_DEBUG
    std::cerr << "VOIPNotify::notifyReceivedVoipVideoCall" << std::endl;
#endif

    emit voipVideoCallReceived(peer_id);
}
