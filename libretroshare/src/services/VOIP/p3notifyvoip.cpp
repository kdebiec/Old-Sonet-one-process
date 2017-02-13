#include "p3notifyvoip.h"

#include <algorithm>

RsNotifyVOIP *rsNotifyVOIP = NULL ;

void p3NotifyVOIP::registerNotifyClient(VOIPNotifyClient *cl)
{
    notifyClients.push_back(cl) ;
}

bool p3NotifyVOIP::unregisterNotifyClient(VOIPNotifyClient *nc)
{
    std::list<VOIPNotifyClient*>::iterator it = std::find(notifyClients.begin(), notifyClients.end(), nc);
    if(it != notifyClients.end())
    {
        notifyClients.erase(it);
        return true;
    }
    else
        return false;
}

#define FOR_ALL_NOTIFY_CLIENTS for(std::list<VOIPNotifyClient*>::const_iterator it(notifyClients.begin());it!=notifyClients.end();++it)


void p3NotifyVOIP::notifyReceivedVoipAccept       (const RsPeerId &peer_id, const uint32_t flags)
    { FOR_ALL_NOTIFY_CLIENTS (*it)->notifyReceivedVoipAccept(peer_id, flags) ; }

void p3NotifyVOIP::notifyReceivedVoipBandwidth    (const RsPeerId &peer_id, uint32_t bytes_per_sec)
    { FOR_ALL_NOTIFY_CLIENTS (*it)->notifyReceivedVoipBandwidth(peer_id, bytes_per_sec) ; }

void p3NotifyVOIP::notifyReceivedVoipData         (const RsPeerId &peer_id)
    { FOR_ALL_NOTIFY_CLIENTS (*it)->notifyReceivedVoipData(peer_id) ; }

void p3NotifyVOIP::notifyReceivedVoipHangUp       (const RsPeerId &peer_id, const uint32_t flags)
    { FOR_ALL_NOTIFY_CLIENTS (*it)->notifyReceivedVoipHangUp (peer_id, flags) ; }

void p3NotifyVOIP::notifyReceivedVoipInvite       (const RsPeerId &peer_id, const uint32_t flags)
    { FOR_ALL_NOTIFY_CLIENTS (*it)->notifyReceivedVoipInvite(peer_id, flags) ; }

void p3NotifyVOIP::notifyReceivedVoipAudioCall    (const RsPeerId &peer_id)
    { FOR_ALL_NOTIFY_CLIENTS (*it)->notifyReceivedVoipAudioCall (peer_id) ; }

void p3NotifyVOIP::notifyReceivedVoipVideoCall    (const RsPeerId &peer_id)
    { FOR_ALL_NOTIFY_CLIENTS (*it)->notifyReceivedVoipVideoCall (peer_id) ; }
