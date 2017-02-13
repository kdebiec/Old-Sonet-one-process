#ifndef P3NOTIFYVOIP_H
#define P3NOTIFYVOIP_H

#include "rsnotifyvoip.h"
#include "util/rsthreads.h"

class p3NotifyVOIP: public RsNotifyVOIP
{
public:
    p3NotifyVOIP() { return; }

    virtual void registerNotifyClient(VOIPNotifyClient *nc);
    virtual bool unregisterNotifyClient(VOIPNotifyClient *nc);

    void notifyReceivedVoipAccept       (const RsPeerId &peer_id, const uint32_t flags);
    void notifyReceivedVoipBandwidth    (const RsPeerId &peer_id, uint32_t bytes_per_sec);
    void notifyReceivedVoipData         (const RsPeerId &peer_id);
    void notifyReceivedVoipHangUp       (const RsPeerId &peer_id, const uint32_t flags);
    void notifyReceivedVoipInvite       (const RsPeerId &peer_id, const uint32_t flags);
    void notifyReceivedVoipAudioCall    (const RsPeerId &peer_id);
    void notifyReceivedVoipVideoCall    (const RsPeerId &peer_id);

private:
    std::list<VOIPNotifyClient*> notifyClients;
};

#endif // P3NOTIFYVOIP_H
