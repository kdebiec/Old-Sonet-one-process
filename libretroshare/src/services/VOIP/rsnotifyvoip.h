#ifndef RSNOTIFYVOIP_H
#define RSNOTIFYVOIP_H

/*libretroshare*/
#include <retroshare/rstypes.h>

// This mechanism is copy-paste of p3Notify in libretroshare

class RsNotifyVOIP;
extern RsNotifyVOIP *rsNotifyVOIP;

class VOIPNotifyClient ;

class RsNotifyVOIP
{
public:
    /* registration of notifies clients */
    virtual void registerNotifyClient(VOIPNotifyClient *nc) = 0;
    /* returns true if NotifyClient was found */
    virtual bool unregisterNotifyClient(VOIPNotifyClient *nc) = 0;
};

class VOIPNotifyClient
{
    public:
        VOIPNotifyClient() {}
        virtual ~VOIPNotifyClient() {}

        virtual void notifyReceivedVoipAccept       (const RsPeerId &peer_id, const uint32_t flags) = 0;
        virtual void notifyReceivedVoipBandwidth    (const RsPeerId &peer_id, uint32_t bytes_per_sec) = 0;
        virtual void notifyReceivedVoipData         (const RsPeerId &peer_id) = 0;
        virtual void notifyReceivedVoipHangUp       (const RsPeerId &peer_id, const uint32_t flags) = 0;
        virtual void notifyReceivedVoipInvite       (const RsPeerId &peer_id, const uint32_t flags) = 0;
        virtual void notifyReceivedVoipAudioCall    (const RsPeerId &peer_id) = 0;
        virtual void notifyReceivedVoipVideoCall    (const RsPeerId &peer_id) = 0;
};

#endif // RSNOTIFYVOIP_H
