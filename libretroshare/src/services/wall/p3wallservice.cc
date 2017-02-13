#include "p3wallservice.h"

#include <retroshare/rsidentity.h>
#include <serialiser/rsgxsitems.h>
#include <algorithm>

#include "retroshare/rsnotify.h"
#include "rsserver/p3face.h"

#define WALL_PROCESS        0x0001
#define PROCESSING_START_PERIOD     0
#define PROCESSING_INC_PERIOD		15

#define	GXSWALLS_SUBSCRIBED_META		1
#define GXSWALLS_UNPROCESSED_SPECIFIC	2
#define GXSWALLS_UNPROCESSED_GENERIC	3

#define WALL_ALL_GROUPS 		0x0011
#define WALL_UNPROCESSED_MSGS	0x0012
#define WALL_ALL_MSGS           0x0013
#define WALL_BG_POST_META		0x0014



namespace RsWall{

RsWall *rsWall = NULL;

const uint32_t WALL_MSG_STORE_PERIOD = 30*24*60*60; // in seconds

const std::string GXS_WALL_APP_NAME = "gxsWall";
const uint16_t GXS_WALL_APP_MAJOR_VERSION  =       1;
const uint16_t GXS_WALL_APP_MINOR_VERSION  =       0;
const uint16_t GXS_WALL_MIN_MAJOR_VERSION  =       1;
const uint16_t GXS_WALL_MIN_MINOR_VERSION  =       0;

// //////////////////////////////////////////////////////////

#define RSGXS_MAX_SERVICE_STRING	1024
bool encodePostCache(std::string &str, const PostStats &s)
{
    char line[RSGXS_MAX_SERVICE_STRING];

    snprintf(line, RSGXS_MAX_SERVICE_STRING, "%d %d %d", s.comments, s.up_votes, s.down_votes);

    str = line;
    return true;
}

bool extractPostCache(const std::string &str, PostStats &s)
{

    uint32_t iupvotes, idownvotes, icomments;
    if (3 == sscanf(str.c_str(), "%d %d %d", &icomments, &iupvotes, &idownvotes))
    {
        s.comments = icomments;
        s.up_votes = iupvotes;
        s.down_votes = idownvotes;
        return true;
    }
    return false;
}

bool PostMsg::calculateScores(time_t ref_time)
{
    /* so we want to calculate all the scores for this Post. */

    PostStats stats;
    extractPostCache(mMeta.mServiceString, stats);

    mUpVotes = stats.up_votes;
    mDownVotes = stats.down_votes;
    mComments = stats.comments;
    mHaveVoted = (mMeta.mMsgStatus & GXS_SERV::GXS_MSG_STATUS_VOTE_MASK);
/*
    time_t age_secs = ref_time - mMeta.mPublishTs;
#define POSTED_AGESHIFT (2.0)
#define POSTED_AGEFACTOR (3600.0)

    mTopScore = ((int) mUpVotes - (int) mDownVotes);
    if (mTopScore > 0)
    {
        // score drops with time.
        mHotScore =  mTopScore / pow(POSTED_AGESHIFT + age_secs / POSTED_AGEFACTOR, 1.5);
    }
    else
    {
        // gets more negative with time.
        mHotScore =  mTopScore * pow(POSTED_AGESHIFT + age_secs / POSTED_AGEFACTOR, 1.5);
    }
    mNewScore = -age_secs;
*/
    return true;
}

// //////////////////////////////////////////////////////////

p3WallService::p3WallService(RsGeneralDataService *gds, RsNetworkExchangeService *nes, RsGixs *gixs)
    : RsGenExchange(gds, nes, new WallSerialiser(), RS_SERVICE_GXS_TYPE_WALLS, gixs, wallAuthPolicy()),
    RsWall(this),
    GxsTokenQueue(this),
    mPostBaseMtx("PostBaseMtx")
{
    mBgProcessing = false;
    mCommentService = new p3GxsCommentService(this,  RS_SERVICE_GXS_TYPE_WALLS);

    RsTickEvent::schedule_in(WALL_PROCESS, PROCESSING_START_PERIOD);
}

RsServiceInfo p3WallService::getServiceInfo()
{
    return RsServiceInfo(RS_SERVICE_GXS_TYPE_WALLS,
            GXS_WALL_APP_NAME,
            GXS_WALL_APP_MAJOR_VERSION,
            GXS_WALL_APP_MINOR_VERSION,
            GXS_WALL_MIN_MAJOR_VERSION,
            GXS_WALL_MIN_MINOR_VERSION);
}

uint32_t p3WallService::wallAuthPolicy()
{
    uint32_t policy = 0;
    uint32_t flag = 0;

    //flag = GXS_SERV::MSG_AUTHEN_ROOT_AUTHOR_SIGN | GXS_SERV::MSG_AUTHEN_CHILD_AUTHOR_SIGN;

    RsGenExchange::setAuthenPolicyFlag(flag, policy, RsGenExchange::PUBLIC_GRP_BITS);
    RsGenExchange::setAuthenPolicyFlag(flag, policy, RsGenExchange::RESTRICTED_GRP_BITS);
    RsGenExchange::setAuthenPolicyFlag(flag, policy, RsGenExchange::PRIVATE_GRP_BITS);

    flag = 0;
    RsGenExchange::setAuthenPolicyFlag(flag, policy, RsGenExchange::GRP_OPTION_BITS);

    return policy;
}

// called by RsGenExchange
void p3WallService::notifyChanges(std::vector<RsGxsNotify*> &changes)
{
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::notifyChanges()";
    std::cerr << std::endl;
#endif

    p3Notify *notify = NULL;
    if (!changes.empty())
    {
        notify = RsServer::notify();
    }

    /* iterate through and grab any new messages */
    std::list<RsGxsGroupId> unprocessedGroups;

    std::vector<RsGxsNotify *>::iterator it;
    for(it = changes.begin(); it != changes.end(); ++it)
    {
        RsGxsMsgChange *msgChange = dynamic_cast<RsGxsMsgChange *>(*it);
        if (msgChange)
        {
            if (msgChange->getType() == RsGxsNotify::TYPE_RECEIVE)
            {
                /* message received */
                if (notify)
                {
                    std::map<RsGxsGroupId, std::vector<RsGxsMessageId> > &msgChangeMap = msgChange->msgChangeMap;
                    std::map<RsGxsGroupId, std::vector<RsGxsMessageId> >::iterator mit;
                    for (mit = msgChangeMap.begin(); mit != msgChangeMap.end(); ++mit)
                    {
                        std::vector<RsGxsMessageId>::iterator mit1;
                        for (mit1 = mit->second.begin(); mit1 != mit->second.end(); ++mit1)
                        {
                            notify->AddFeedItem(RS_FEED_ITEM_WALL_MSG, mit->first.toStdString(), mit1->toStdString());
                        }
                    }
                }
            }
            if (!msgChange->metaChange())
            {
#ifdef GXSWALLS_DEBUG
                std::cerr << "p3WallService::notifyChanges() Found Message Change Notification";
                std::cerr << std::endl;
#endif

                std::map<RsGxsGroupId, std::vector<RsGxsMessageId> > &msgChangeMap = msgChange->msgChangeMap;
                std::map<RsGxsGroupId, std::vector<RsGxsMessageId> >::iterator mit;
                for(mit = msgChangeMap.begin(); mit != msgChangeMap.end(); ++mit)
                {
#ifdef GXSWALLS_DEBUG
                    std::cerr << "p3WallService::notifyChanges() Msgs for Group: " << mit->first;
                    std::cerr << std::endl;
#endif
                    unprocessedGroups.push_back(mit->first);
                }
            }
        }
        else
        {
            if (notify)
            {
                RsGxsGroupChange *grpChange = dynamic_cast<RsGxsGroupChange*>(*it);
                if (grpChange)
                {
                    switch (grpChange->getType())
                    {
                        case RsGxsNotify::TYPE_PROCESSED:
                        case RsGxsNotify::TYPE_PUBLISH:
                            break;

                        case RsGxsNotify::TYPE_RECEIVE:
                        {
                            /* group received */
                            std::list<RsGxsGroupId> &grpList = grpChange->mGrpIdList;
                            std::list<RsGxsGroupId>::iterator git;
                            for (git = grpList.begin(); git != grpList.end(); ++git)
                            {
                                notify->AddFeedItem(RS_FEED_ITEM_WALL_NEW, git->toStdString());
                            }
                            break;
                        }

                        case RsGxsNotify::TYPE_PUBLISHKEY:
                        {
                            /* group received */
                            std::list<RsGxsGroupId> &grpList = grpChange->mGrpIdList;
                            std::list<RsGxsGroupId>::iterator git;
                            for (git = grpList.begin(); git != grpList.end(); ++git)
                            {
                                notify->AddFeedItem(RS_FEED_ITEM_WALL_PUBLISHKEY, git->toStdString());
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    //request_SpecificSubscribedGroups(unprocessedGroups);

    // forward the changes
    RsGxsIfaceHelper::receiveChanges(changes);
}

// called by RsGenExchange
void p3WallService::service_tick()
{
    RsTickEvent::tick_events();
    GxsTokenQueue::checkRequests();
}

void p3WallService::handleResponse(uint32_t token, uint32_t req_type)
{
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::handleResponse(" << token << "," << req_type << ")" << std::endl;
#endif

    switch(req_type)
    {
        case WALL_ALL_GROUPS:
            background_loadGroups(token);
            break;

        case WALL_UNPROCESSED_MSGS:
            background_loadUnprocessedMsgs(token);
            break;

        case WALL_ALL_MSGS:
            background_loadAllMsgs(token);
            break;

        case WALL_BG_POST_META:
            background_updateVoteCounts(token);
            break;

        default:
            /* error */

            std::cerr << "p3WallService::handleResponse() Unknown Request Type: " << req_type;
            std::cerr << std::endl;
            break;
    }
}

bool p3WallService::getWallGroups(const uint32_t &token, std::vector<WallGroup> &groups)
{
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::getWallGroups()" << std::endl;
#endif

    std::vector<RsGxsGrpItem*> grpData;
    bool ok = RsGenExchange::getGroupData(token, grpData);

    if(ok)
    {
        std::vector<RsGxsGrpItem*>::iterator vit = grpData.begin();

        for(; vit != grpData.end(); ++vit)
        {
            WallGroupItem* item = dynamic_cast<WallGroupItem*>(*vit);
            if (item)
            {
                WallGroup grp;
                item->toWallGroup(grp);
                delete item;
                groups.push_back(grp);
            }
            else
            {
                std::cerr << "p3WallService::getWallGroups() ERROR in decode";
                std::cerr << std::endl;
                delete(*vit);
            }
        }
    }
    else
    {
        std::cerr << "p3WallService::getWallGroups() ERROR in request";
        std::cerr << std::endl;
    }

    return ok;
}

void p3WallService::createWallGroup(uint32_t &token, const WallGroup &grp)
{
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::createWallGroup()" << std::endl;
#endif

    WallGroupItem* grpItem = new WallGroupItem();
    grpItem->fromWallGroup(grp);
    RsGenExchange::publishGroup(token, grpItem);
}

void p3WallService::updateWallGroup(uint32_t &token, const WallGroup &grp)
{
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::updateWallGroup()" << std::endl;
#endif

    WallGroupItem* grpItem = new WallGroupItem();
    grpItem->fromWallGroup(grp);
    RsGenExchange::updateGroup(token, grpItem);
}

bool p3WallService::createPost(uint32_t &token, PostMsg &post)
{
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::createPost() GroupId: " << post.mMeta.mGroupId;
    std::cerr << std::endl;
#endif

    PostMsgItem* msgItem = new PostMsgItem();
    msgItem->fromWallPost(post);

    RsGenExchange::publishMsg(token, msgItem);
    return true;
}

bool p3WallService::getPostData(const uint32_t &token, std::vector<PostMsg> &posts)
{
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::getPostData()";
    std::cerr << std::endl;
#endif

    GxsMsgDataMap msgData;
    bool ok = RsGenExchange::getMsgData(token, msgData);
    time_t now = time(NULL);

    if(ok)
    {
        GxsMsgDataMap::iterator mit = msgData.begin();

        for(; mit != msgData.end();  ++mit)
        {
            std::vector<RsGxsMsgItem*>& msgItems = mit->second;
            std::vector<RsGxsMsgItem*>::iterator vit = msgItems.begin();

            for(; vit != msgItems.end(); ++vit)
            {
                PostMsgItem* postItem = dynamic_cast<PostMsgItem*>(*vit);

                if(postItem)
                {
                    PostMsg msg;
                    postItem->toWallPost(msg);
                    msg.calculateScores(now);

                    posts.push_back(msg);
                    delete postItem;
                }
                else
                {
                    std::cerr << "Not a PostMsgItem, deleting!" << std::endl;
                    delete *vit;
                }
            }
        }
    }
    else
    {
        std::cerr << "p3WallService::getPostData() ERROR in request";
        std::cerr << std::endl;
    }

    return ok;
}


    /*********************************************************************************
     * Background Calculations.
     */

void p3WallService::handle_event(uint32_t event_type, const std::string &elabel)
{
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::handle_event(" << event_type << ")" << std::endl;
#endif

    // stuff.
    switch(event_type)
    {
        case WALL_PROCESS:
            background_tick();
            //request_AllSubscribedGroups();
            break;

        default:
            /* error */
            std::cerr << "p3WallService::handle_event() Unknown Event Type: " << event_type << " elabel:" << elabel;
            std::cerr << std::endl;
            break;
    }
}

void p3WallService::background_tick()
{

#if 1
    {
        RsStackMutex stack(mPostBaseMtx); /********** STACK LOCKED MTX ******/
        if (mBgGroupList.empty())
        {
            background_requestAllGroups();
        }
    }
#endif

    background_requestUnprocessedGroup();

    RsTickEvent::schedule_in(WALL_PROCESS, PROCESSING_INC_PERIOD);

}

bool p3WallService::background_requestAllGroups()
{
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::background_requestAllGroups()";
    std::cerr << std::endl;
#endif

    uint32_t ansType = RS_TOKREQ_ANSTYPE_LIST;
    RsTokReqOptions opts;
    opts.mReqType = GXS_REQUEST_TYPE_GROUP_IDS;

    uint32_t token = 0;
    RsGenExchange::getTokenService()->requestGroupInfo(token, ansType, opts);
    GxsTokenQueue::queueRequest(token, WALL_ALL_GROUPS);

    return true;
}


void p3WallService::background_loadGroups(const uint32_t &token)
{
    /* get messages */
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::background_loadGroups()";
    std::cerr << std::endl;
#endif

    std::list<RsGxsGroupId> groupList;
    bool ok = RsGenExchange::getGroupList(token, groupList);

    if (!ok)
    {
        return;
    }

    std::list<RsGxsGroupId>::iterator it;
    for(it = groupList.begin(); it != groupList.end(); ++it)
    {
        addGroupForProcessing(*it);
    }
}


void p3WallService::addGroupForProcessing(RsGxsGroupId grpId)
{
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::addGroupForProcessing(" << grpId << ")";
    std::cerr << std::endl;
#endif // GXSWALLS_DEBUG

    {
        RsStackMutex stack(mPostBaseMtx); /********** STACK LOCKED MTX ******/
        // no point having multiple lookups queued.
        if (mBgGroupList.end() == std::find(mBgGroupList.begin(),
                        mBgGroupList.end(), grpId))
        {
            mBgGroupList.push_back(grpId);
        }
    }
}


void p3WallService::background_requestUnprocessedGroup()
{
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::background_requestUnprocessedGroup()";
    std::cerr << std::endl;
#endif // GXSWALLS_DEBUG


    RsGxsGroupId grpId;
    {
        RsStackMutex stack(mPostBaseMtx); /********** STACK LOCKED MTX ******/
        if (mBgProcessing)
        {
#ifdef GXSWALLS_DEBUG
            std::cerr << "p3WallService::background_requestUnprocessedGroup() Already Active";
            std::cerr << std::endl;
#endif
            return;
        }
        if (mBgGroupList.empty())
        {
#ifdef GXSWALLS_DEBUG
            std::cerr << "p3WallService::background_requestUnprocessedGroup() No Groups to Process";
            std::cerr << std::endl;
#endif
            return;
        }

        grpId = mBgGroupList.front();
        mBgGroupList.pop_front();
        mBgProcessing = true;
    }

    background_requestGroupMsgs(grpId, true);
}

void p3WallService::background_requestGroupMsgs(const RsGxsGroupId &grpId, bool unprocessedOnly)
{
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::background_requestGroupMsgs() id: " << grpId;
    std::cerr << std::endl;
#endif

    uint32_t ansType = RS_TOKREQ_ANSTYPE_DATA;
    RsTokReqOptions opts;
    opts.mReqType = GXS_REQUEST_TYPE_MSG_DATA;

    if (unprocessedOnly)
    {
        opts.mStatusFilter = GXS_SERV::GXS_MSG_STATUS_UNPROCESSED;
        opts.mStatusMask = GXS_SERV::GXS_MSG_STATUS_UNPROCESSED;
    }

    std::list<RsGxsGroupId> grouplist;
    grouplist.push_back(grpId);

    uint32_t token = 0;

    RsGenExchange::getTokenService()->requestMsgInfo(token, ansType, opts, grouplist);

    if (unprocessedOnly)
    {
        GxsTokenQueue::queueRequest(token, WALL_UNPROCESSED_MSGS);
    }
    else
    {
        GxsTokenQueue::queueRequest(token, WALL_ALL_MSGS);
    }
}

void p3WallService::background_loadUnprocessedMsgs(const uint32_t &token)
{
    background_loadMsgs(token, true);
}


void p3WallService::background_loadAllMsgs(const uint32_t &token)
{
    background_loadMsgs(token, false);
}


/* This function is generalised to support any collection of messages, across multiple groups */

void p3WallService::background_loadMsgs(const uint32_t &token, bool unprocessed)
{
    /* get messages */
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::background_loadMsgs()";
    std::cerr << std::endl;
#endif

    std::map<RsGxsGroupId, std::vector<RsGxsMsgItem*> > msgData;
    bool ok = RsGenExchange::getMsgData(token, msgData);

    if (!ok)
    {
        std::cerr << "p3WallService::background_loadMsgs() Failed to getMsgData()";
        std::cerr << std::endl;

        /* cleanup */
        background_cleanup();
        return;

    }

    {
        RsStackMutex stack(mPostBaseMtx); /********** STACK LOCKED MTX ******/
        mBgStatsMap.clear();
        mBgIncremental = unprocessed;
    }

    std::map<RsGxsGroupId, std::vector<RsGxsMessageId> > postMap;

    // generate vector of changes to push to the GUI.
    std::vector<RsGxsNotify *> changes;
    RsGxsMsgChange *msgChanges = new RsGxsMsgChange(RsGxsNotify::TYPE_PROCESSED, false);


    RsGxsGroupId groupId;
    std::map<RsGxsGroupId, std::vector<RsGxsMsgItem*> >::iterator mit;
    std::vector<RsGxsMsgItem*>::iterator vit;
    for (mit = msgData.begin(); mit != msgData.end(); ++mit)
    {
          groupId = mit->first;
          for (vit = mit->second.begin(); vit != mit->second.end(); ++vit)
          {
            RsGxsMessageId parentId = (*vit)->meta.mParentId;
            RsGxsMessageId threadId = (*vit)->meta.mThreadId;


            bool inc_counters = false;
            uint32_t vote_up_inc = 0;
            uint32_t vote_down_inc = 0;
            uint32_t comment_inc = 0;

            bool add_voter = false;
            RsGxsId voterId;
            RsGxsCommentItem *commentItem;
            RsGxsVoteItem    *voteItem;

            /* THIS Should be handled by UNPROCESSED Filter - but isn't */
            if (!IS_MSG_UNPROCESSED((*vit)->meta.mMsgStatus))
            {
                RsStackMutex stack(mPostBaseMtx); /********** STACK LOCKED MTX ******/
                if (mBgIncremental)
                {
#ifdef GXSWALLS_DEBUG
                    std::cerr << "p3WallService::background_loadMsgs() Msg already Processed - Skipping";
                    std::cerr << std::endl;
                    std::cerr << "p3WallService::background_loadMsgs() ERROR This should not happen";
                    std::cerr << std::endl;
#endif
                    delete(*vit);
                    continue;
                }
            }

            /* 3 types expected: PostedPost, Comment and Vote */
            if (parentId.isNull())
            {
#ifdef GXSWALLS_DEBUG
                /* we don't care about top-level (Posts) */
                std::cerr << "\tIgnoring TopLevel Item";
                std::cerr << std::endl;
#endif

                /* but we need to notify GUI about them */
                msgChanges->msgChangeMap[mit->first].push_back((*vit)->meta.mMsgId);
            }
            else if (NULL != (commentItem = dynamic_cast<RsGxsCommentItem *>(*vit)))
            {
#ifdef GXSWALLS_DEBUG
                /* comment - want all */
                /* Comments are counted by Thread Id */
                std::cerr << "\tProcessing Comment: " << commentItem;
                std::cerr << std::endl;
#endif

                inc_counters = true;
                comment_inc = 1;
            }
            else if (NULL != (voteItem = dynamic_cast<RsGxsVoteItem *>(*vit)))
            {
                /* vote - only care about direct children */
                if (parentId == threadId)
                {
                    /* Votes are organised by Parent Id,
                     * ie. you can vote for both Posts and Comments
                     */
#ifdef GXSWALLS_DEBUG
                    std::cerr << "\tProcessing Vote: " << voteItem;
                    std::cerr << std::endl;
#endif

                    inc_counters = true;
                    add_voter = true;
                    voterId = voteItem->meta.mAuthorId;

                    if (voteItem->mMsg.mVoteType == GXS_VOTE_UP)
                    {
                        vote_up_inc = 1;
                    }
                    else
                    {
                        vote_down_inc = 1;
                    }
                }
            }
            else
            {
                /* unknown! */
                std::cerr << "p3WallService::background_processNewMessages() ERROR Strange NEW Message:";
                std::cerr << std::endl;
                std::cerr << "\t" << (*vit)->meta;
                std::cerr << std::endl;

            }

            if (inc_counters)
            {
                RsStackMutex stack(mPostBaseMtx); /********** STACK LOCKED MTX ******/

                std::map<RsGxsMessageId, PostStats>::iterator sit = mBgStatsMap.find(threadId);
                if (sit == mBgStatsMap.end())
                {
                    // add to map of ones to update.
                    postMap[groupId].push_back(threadId);

                    mBgStatsMap[threadId] = PostStats(0,0,0);
                    sit = mBgStatsMap.find(threadId);
                }

                sit->second.comments += comment_inc;
                sit->second.up_votes += vote_up_inc;
                sit->second.down_votes += vote_down_inc;


                if (add_voter)
                {
                    sit->second.voters.push_back(voterId);
                }

#ifdef GXSWALLS_DEBUG
                std::cerr << "\tThreadId: " << threadId;
                std::cerr << " Comment Total: " << sit->second.comments;
                std::cerr << " UpVote Total: " << sit->second.up_votes;
                std::cerr << " DownVote Total: " << sit->second.down_votes;
                std::cerr << std::endl;
#endif
            }

            /* flag all messages as processed and new for the gui */
            if ((*vit)->meta.mMsgStatus & GXS_SERV::GXS_MSG_STATUS_UNPROCESSED)
            {
                uint32_t token_a;
                RsGxsGrpMsgIdPair msgId = std::make_pair(groupId, (*vit)->meta.mMsgId);
                RsGenExchange::setMsgStatusFlags(token_a, msgId, GXS_SERV::GXS_MSG_STATUS_GUI_NEW | GXS_SERV::GXS_MSG_STATUS_GUI_UNREAD, GXS_SERV::GXS_MSG_STATUS_UNPROCESSED | GXS_SERV::GXS_MSG_STATUS_GUI_NEW | GXS_SERV::GXS_MSG_STATUS_GUI_UNREAD);
            }
            delete(*vit);
        }
    }

    /* push updates of new Posts */
    if (msgChanges->msgChangeMap.size() > 0)
    {
#ifdef GXSWALLS_DEBUG
        std::cerr << "p3WallService::background_processNewMessages() -> receiveChanges()";
        std::cerr << std::endl;
#endif

        changes.push_back(msgChanges);
        receiveHelperChanges(changes);
    }
    else
    {
        delete(msgChanges);
    }

    /* request the summary info from the parents */
    uint32_t token_b;
    uint32_t anstype = RS_TOKREQ_ANSTYPE_SUMMARY;
    RsTokReqOptions opts;
    opts.mReqType = GXS_REQUEST_TYPE_MSG_META;
    RsGenExchange::getTokenService()->requestMsgInfo(token_b, anstype, opts, postMap);

    GxsTokenQueue::queueRequest(token_b, WALL_BG_POST_META);
    return;
}

void p3WallService::background_updateVoteCounts(const uint32_t &token)
{
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::background_updateVoteCounts()" << std::endl;
#endif

    GxsMsgMetaMap parentMsgList;
    GxsMsgMetaMap::iterator mit;
    std::vector<RsMsgMetaData>::iterator vit;

    bool ok = RsGenExchange::getMsgMeta(token, parentMsgList);

    if (!ok)
    {
#ifdef GXSWALLS_DEBUG
        std::cerr << "p3WallService::background_updateVoteCounts() ERROR" << std::endl;
#endif
        background_cleanup();
        return;
    }

    // generate vector of changes to push to the GUI.
    std::vector<RsGxsNotify *> changes;
    RsGxsMsgChange *msgChanges = new RsGxsMsgChange(RsGxsNotify::TYPE_PROCESSED, false);

    for(mit = parentMsgList.begin(); mit != parentMsgList.end(); ++mit)
    {
        for(vit = mit->second.begin(); vit != mit->second.end(); ++vit)
        {
#ifdef GXSWALLS_DEBUG
            std::cerr << "p3WallService::background_updateVoteCounts() Processing Msg(" << mit->first;
            std::cerr << ", " << vit->mMsgId << ")" << std::endl;
#endif

            RsStackMutex stack(mPostBaseMtx); /********** STACK LOCKED MTX ******/

            /* extract current vote count */
            PostStats stats;
            if (mBgIncremental)
            {
                if (!extractPostCache(vit->mServiceString, stats))
                {
                    if (!(vit->mServiceString.empty()))
                    {
                        std::cerr << "p3WallService::background_updateVoteCounts() Failed to extract Votes";
                        std::cerr << std::endl;
                        std::cerr << "\tFrom String: " << vit->mServiceString;
                        std::cerr << std::endl;
                    }
                }
            }

            /* get increment */
            std::map<RsGxsMessageId, PostStats>::iterator it;
            it = mBgStatsMap.find(vit->mMsgId);

            if (it != mBgStatsMap.end())
            {
#ifdef GXSWALLS_DEBUG
                std::cerr << "p3WallService::background_updateVoteCounts() Adding to msgChangeMap: ";
                std::cerr << mit->first << " MsgId: " << vit->mMsgId;
                std::cerr << std::endl;
#endif

                stats.increment(it->second);
                msgChanges->msgChangeMap[mit->first].push_back(vit->mMsgId);
            }
            else
            {
#ifdef GXSWALLS_DEBUG
                // warning.
                std::cerr << "p3WallService::background_updateVoteCounts() Warning No New Votes found.";
                std::cerr << " For MsgId: " << vit->mMsgId;
                std::cerr << std::endl;
#endif
            }

            std::string str;
            if (!encodePostCache(str, stats))
            {
                std::cerr << "p3WallService::background_updateVoteCounts() Failed to encode Votes";
                std::cerr << std::endl;
            }
            else
            {
#ifdef GXSWALLS_DEBUG
                std::cerr << "p3WallService::background_updateVoteCounts() Encoded String: " << str;
                std::cerr << std::endl;
#endif
                /* store new result */
                uint32_t token_c;
                RsGxsGrpMsgIdPair msgId = std::make_pair(vit->mGroupId, vit->mMsgId);
                RsGenExchange::setMsgServiceString(token_c, msgId, str);
            }
        }
    }

    if (msgChanges->msgChangeMap.size() > 0)
    {
#ifdef GXSWALLS_DEBUG
        std::cerr << "p3WallService::background_updateVoteCounts() -> receiveChanges()";
        std::cerr << std::endl;
#endif

        changes.push_back(msgChanges);
        receiveHelperChanges(changes);
    }
    else
    {
        delete(msgChanges);
    }

    // DONE!.
    background_cleanup();
    return;
}

bool p3WallService::background_cleanup()
{
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::background_cleanup()";
    std::cerr << std::endl;
#endif

    RsStackMutex stack(mPostBaseMtx); /********** STACK LOCKED MTX ******/

    // Cleanup.
    mBgStatsMap.clear();
    mBgProcessing = false;

    return true;
}





/*******************************************************************
 *
 */



/*

   //Overloaded to cache new groups
RsGenExchange::ServiceCreate_Return p3WallService::service_CreateGroup(RsGxsGrpItem* grpItem, RsTlvSecurityKeySet& / * keySet * /)
{
    updateSubscribedGroup(grpItem->meta);
    return SERVICE_CREATE_SUCCESS;
}

void p3WallService::updateSubscribedGroup(const RsGroupMetaData &group)
{
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::updateSubscribedGroup() id: " << group.mGroupId;
    std::cerr << std::endl;
#endif

    mSubscribedGroups[group.mGroupId] = group;
}

void p3WallService::request_SpecificSubscribedGroups(const std::list<RsGxsGroupId> &groups)
{
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::request_SpecificSubscribedGroups()" << std::endl;
#endif

    uint32_t ansType = RS_TOKREQ_ANSTYPE_SUMMARY;
    RsTokReqOptions opts;
    opts.mReqType = GXS_REQUEST_TYPE_GROUP_META;

    uint32_t token = 0;

    RsGenExchange::getTokenService()->requestGroupInfo(token, ansType, opts, groups);
    GxsTokenQueue::queueRequest(token, GXSWALLS_SUBSCRIBED_META);
}

void p3WallService::request_AllSubscribedGroups()
{
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::request_SubscribedGroups()" << std::endl;
#endif

    uint32_t ansType = RS_TOKREQ_ANSTYPE_SUMMARY;
    RsTokReqOptions opts;
    opts.mReqType = GXS_REQUEST_TYPE_GROUP_META;

    uint32_t token = 0;

    RsGenExchange::getTokenService()->requestGroupInfo(token, ansType, opts);
    GxsTokenQueue::queueRequest(token, GXSWALLS_SUBSCRIBED_META);

#define PERIODIC_ALL_PROCESS	60 // TESTING every 1 minutes.
    RsTickEvent::schedule_in(WALL_PROCESS, PERIODIC_ALL_PROCESS);
}

void p3WallService::load_SubscribedGroups(const uint32_t &token)
{
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::load_SubscribedGroups()" << std::endl;
#endif

    std::list<RsGroupMetaData> groups;
    getGroupMeta(token, groups);

    std::list<RsGroupMetaData>::iterator it;
    for(it = groups.begin(); it != groups.end(); ++it)
    {
        if (it->mSubscribeFlags &
            (GXS_SERV::GROUP_SUBSCRIBE_ADMIN |
            GXS_SERV::GROUP_SUBSCRIBE_PUBLISH |
            GXS_SERV::GROUP_SUBSCRIBE_SUBSCRIBED ))
        {
#ifdef GXSWALLS_DEBUG
            std::cerr << "p3WallService::load_SubscribedGroups() updating Subscribed Group: " << it->mGroupId << std::endl;
#endif

            updateSubscribedGroup(*it);
        }
        else
        {
#ifdef GXSWALLS_DEBUG
            std::cerr << "p3WallService::load_SubscribedGroups() clearing unsubscribed Group: " << it->mGroupId << std::endl;
#endif
            clearUnsubscribedGroup(it->mGroupId);
        }
    }
}

void p3WallService::clearUnsubscribedGroup(const RsGxsGroupId &id)
{
#ifdef GXSWALLS_DEBUG
    std::cerr << "p3WallService::clearUnsubscribedGroup() id: " << id;
    std::cerr << std::endl;
#endif

    std::map<RsGxsGroupId, RsGroupMetaData>::iterator it;

    it = mSubscribedGroups.find(id);
    if (it != mSubscribedGroups.end())
    {
        mSubscribedGroups.erase(it);
    }
}*/
}//namespace RsWall
