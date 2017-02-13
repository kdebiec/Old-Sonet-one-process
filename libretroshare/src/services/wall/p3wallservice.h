#pragma once

#include <gxs/rsgenexchange.h>
#include <gxs/gxstokenqueue.h>
#include "services/p3gxscommon.h"
#include "rswall.h"
#include "rswallitems.h"
#include "util/rstickevent.h"

//#include <typeinfo>

namespace RsWall{

class PostStats
{
    public:
    PostStats() :up_votes(0), down_votes(0), comments(0) { return; }
    PostStats(int up, int down, int c) :up_votes(up), down_votes(down), comments(c) { return; }

    void increment(const PostStats &s)
    {
        up_votes += s.up_votes;
        down_votes += s.down_votes;
        comments += s.comments;
        return;
    }

    int up_votes;
    int down_votes;
    int comments;
    std::list<RsGxsId> voters;
};

bool encodePostCache(std::string &str, const PostStats &s);
bool extractPostCache(const std::string &str, PostStats &s);


// have two threads: ui thread, rsgenexchange thread
// have to be careful when they cross their ways
class p3WallService: public RsWall, public RsGenExchange, public GxsTokenQueue,
        public RsTickEvent	/* only needed for testing - remove after */
{
public:
    p3WallService(RsGeneralDataService* gds, RsNetworkExchangeService* nes, RsGixs* gixs);

    // from RsGenExchange
    virtual RsServiceInfo getServiceInfo();
    virtual void service_tick();

public:

    virtual void createWallGroup(uint32_t &token, const WallGroup &grp);
    virtual void updateWallGroup(uint32_t &token, const WallGroup &grp);
    virtual bool getWallGroups(const uint32_t &token, std::vector<WallGroup> &wgs);

    virtual bool createPost(uint32_t &token, PostMsg &post);
    virtual bool getPostData(const uint32_t &token, std::vector<PostMsg> &posts);

    // Overloaded from RsGxsIface.
    //virtual bool subscribeToGroup(uint32_t &token, const RsGxsGroupId &groupId, bool subscribe);

    /* Comment service - Provide RsGxsCommentService - redirect to p3GxsCommentService */
    virtual bool getCommentData(const uint32_t &token, std::vector<RsGxsComment> &msgs)
    {
            return mCommentService->getGxsCommentData(token, msgs);
    }

    virtual bool getRelatedComments(const uint32_t &token, std::vector<RsGxsComment> &msgs)
    {
        return mCommentService->getGxsRelatedComments(token, msgs);
    }

    virtual bool createComment(uint32_t &token, RsGxsComment &msg)
    {
        return mCommentService->createGxsComment(token, msg);
    }

    virtual bool createVote(uint32_t &token, RsGxsVote &msg)
    {
        return mCommentService->createGxsVote(token, msg);
    }

    virtual bool acknowledgeComment(const uint32_t& token, std::pair<RsGxsGroupId, RsGxsMessageId>& msgId)
    {
        return acknowledgeMsg(token, msgId);
    }

    virtual bool acknowledgeVote(const uint32_t& token, std::pair<RsGxsGroupId, RsGxsMessageId>& msgId)
    {
        if (mCommentService->acknowledgeVote(token, msgId))
        {
            return true;
        }
        return acknowledgeMsg(token, msgId);
    }

protected:

    // Overloaded to cache new groups.
    //virtual RsGenExchange::ServiceCreate_Return service_CreateGroup(RsGxsGrpItem* grpItem, RsTlvSecurityKeySet& keySet);
    virtual void notifyChanges(std::vector<RsGxsNotify*>& changes);
    // Overloaded from RsTickEvent.
    virtual void handle_event(uint32_t event_type, const std::string &elabel);
    // from GxsTokenQueue
    virtual void handleResponse(uint32_t token, uint32_t req_type);

    virtual void receiveHelperChanges(std::vector<RsGxsNotify*>& changes)
    {
        return RsGxsIfaceHelper::receiveChanges(changes);
    }

private:

    static uint32_t wallAuthPolicy();

    // Local Cache of Subscribed Groups.
    //void updateSubscribedGroup(const RsGroupMetaData &group);
    //void clearUnsubscribedGroup(const RsGxsGroupId &id);
    //std::map<RsGxsGroupId, RsGroupMetaData> mSubscribedGroups;

    // Handle Processing.
    //void request_AllSubscribedGroups();
    //void request_SpecificSubscribedGroups(const std::list<RsGxsGroupId> &groups);
    //void load_SubscribedGroups(const uint32_t &token);

    // Background processing.
    void background_tick();

    bool background_requestAllGroups();
    void background_loadGroups(const uint32_t &token);

    void addGroupForProcessing(RsGxsGroupId grpId);
    void background_requestUnprocessedGroup();

    void background_requestGroupMsgs(const RsGxsGroupId &grpId, bool unprocessedOnly);
    void background_loadUnprocessedMsgs(const uint32_t &token);
    void background_loadAllMsgs(const uint32_t &token);
    void background_loadMsgs(const uint32_t &token, bool unprocessed);

    void background_updateVoteCounts(const uint32_t &token);
    bool background_cleanup();

    p3GxsCommentService *mCommentService;

    RsMutex mPostBaseMtx;

    bool mBgProcessing;
    bool mBgIncremental;
        std::list<RsGxsGroupId> mBgGroupList;
        std::map<RsGxsMessageId, PostStats> mBgStatsMap;
};

} // namespace RsWall
