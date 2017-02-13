#pragma once

#include <retroshare/rsgxsifacehelper.h>
#include <retroshare/rsgxscommon.h>

/* the life of a token
static const uint8_t GXS_REQUEST_V2_STATUS_FAILED;      end if the request failed
static const uint8_t GXS_REQUEST_V2_STATUS_PENDING;     token was just created
static const uint8_t GXS_REQUEST_V2_STATUS_PARTIAL;
static const uint8_t GXS_REQUEST_V2_STATUS_FINISHED_INCOMPLETE;     unused
static const uint8_t GXS_REQUEST_V2_STATUS_COMPLETE;    the request was completed and data is available
static const uint8_t GXS_REQUEST_V2_STATUS_DONE;        the client received the data

*/

namespace RsWall{

// todo
class NewsfeedEntry;

class RsWall;
extern RsWall *rsWall;

// there is a potential name collison with WRasterImage on Ubuntu 14
// so can't call it Image, have to call it WallImage
class WallImage;
class WallGroup;
class PostGroup;
class ReferenceMsg;
class PostMsg;

class PostReferenceParams
{
public:
    PostReferenceParams(): mType(0){}

    RsGxsGroupId mReferencedGroupId;
    RsGxsId mAuthor;
    RsGxsId mTargetWallOwner;
    RsGxsCircleId mCircle;
    uint32_t mType;
};

// to show how others interact with content
// later want to display other activities like "added a friend", "published a photo album"
class Activity
{
public:
    // who did what?
    std::vector<RsGxsId> mShared;
    std::vector<RsGxsId> mCommented;
    //std::vector<RsGxsId> mLiked;

    // id of a post group
    RsGxsGroupId mReferencedGroup;
};

// ********** interface *************************
class RsWall: public RsGxsIfaceHelper, public RsGxsCommentService
{
public:
    RsWall(RsGxsIface *gxs)
    :RsGxsIfaceHelper(gxs)  { return; }

    virtual void createWallGroup(uint32_t &token, const WallGroup &grp) = 0;
    virtual void updateWallGroup(uint32_t &token, const WallGroup &grp) = 0;
    virtual bool getWallGroups(const uint32_t &token, std::vector<WallGroup> &wgs) = 0;

    virtual bool createPost(uint32_t &token, PostMsg &post) = 0;
    virtual bool getPostData(const uint32_t &token, std::vector<PostMsg> &posts) = 0;

    //virtual bool subscribeToGroup(uint32_t &token, const RsGxsGroupId &groupId, bool subscribe) = 0;
};

// RsGxsImage is to complicated
// this class is much simpler, because the memory managment is hidden in a std::vector
    class WallImage{
    public:
        std::vector<uint8_t> mData;
    };

    class WallGroup{
    public:
        RsGroupMetaData mMeta;

        std::string mProfileText;
        WallImage mAvatarImage;
        WallImage mWallImage;
    };

    class PostGroup{
    public:
        RsGroupMetaData mMeta;
    };

    class ReferenceMsg{
    public:
        RsMsgMetaData mMeta;
        // in meta:
        // mAuthorId (optional)
        RsGxsGroupId mReferencedGroup; // this can be a id of a PostGroup or WallGroup
                                       // or even id of a photo album
        static const uint32_t REFTYPE_SHARE = 0;
        static const uint32_t REFTYPE_COMMENT = 1;
        uint32_t mType;
    };

    class PostMsg{
    public:
        PostMsg()
        {
            mUpVotes = 0;
            mDownVotes = 0;
            mComments = 0;
            mHaveVoted = false;
        }

        bool calculateScores(time_t ref_time);

        RsMsgMetaData mMeta;
        // in meta:
        // mGroupId // when creating a new post msg, this represents the target wall id
        //          // when reading it is the correct parent grp id as usual
        // mAuthorId (optional, but same as in PostGroup)
        std::string mPostText;

        bool     mHaveVoted;

        // Calculated.
        uint32_t mUpVotes;
        uint32_t mDownVotes;
        uint32_t mComments;

        // and Calculated Scores:???
        //double  mHotScore;
        //double  mTopScore;
        //double  mNewScore;
    };

    class RsWallMsg{
    public:
        RsMsgMetaData mMeta;
        std::string mMsg;
    };

    class RsWallGroup{
    public:
        RsGroupMetaData mMeta;
    };
} // namespace RsWall
