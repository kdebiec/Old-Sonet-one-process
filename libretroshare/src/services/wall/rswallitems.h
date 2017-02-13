#pragma once

#include <serialiser/rsgxscommentitems.h>
#include <serialiser/rsgxsitems.h>
#include "rswall.h"

namespace RsWall{

const uint8_t RS_PKT_SUBTYPE_WALL_WALL_GRP_ITEM = 0x00;
const uint8_t RS_PKT_SUBTYPE_WALL_POST_GRP_ITEM = 0x01;
const uint8_t RS_PKT_SUBTYPE_WALL_REF_MSG_ITEM  = 0x02;
const uint8_t RS_PKT_SUBTYPE_WALL_POST_MSG_ITEM = 0x03;

// see rsgxscommentitems.h
//const uint8_t RS_PKT_SUBTYPE_GXSCOMMENT_COMMENT_ITEM = 0xf1;
//const uint8_t RS_PKT_SUBTYPE_GXSCOMMENT_VOTE_ITEM = 0xf2;

//*************** item classes ***********************
class WallGroupItem: public RsGxsGrpItem
{
public:
    WallGroupItem():
        RsGxsGrpItem(RS_SERVICE_GXS_TYPE_WALLS, RS_PKT_SUBTYPE_WALL_WALL_GRP_ITEM) {}
    virtual std::ostream &print(std::ostream &out, uint16_t indent = 0);
    virtual void clear();

    // use conversion functions to transform:
    bool fromWallGroup(const WallGroup &group);
    bool toWallGroup(WallGroup &group);

    //WallGroup mWallGroup;
    //RsGroupMetaData mMeta;

    std::string mProfileText;
    WallImage mAvatarImage;
    WallImage mWallImage;
};

class PostMsgItem: public RsGxsMsgItem
{
public:
    PostMsgItem():
        RsGxsMsgItem(RS_SERVICE_GXS_TYPE_WALLS, RS_PKT_SUBTYPE_WALL_POST_MSG_ITEM) {}
    virtual std::ostream &print(std::ostream &out, uint16_t indent = 0);
    virtual void clear();

    // use conversion functions to transform:
    bool fromWallPost(PostMsg &post);
    bool toWallPost(PostMsg &post);
    std::string mPostText;
};

class PostGroupItem: public RsGxsGrpItem
{
public:
    PostGroupItem():
        RsGxsGrpItem(RS_SERVICE_GXS_TYPE_WALLS, RS_PKT_SUBTYPE_WALL_POST_GRP_ITEM) {}
    virtual std::ostream &print(std::ostream &out, uint16_t indent = 0);
    virtual void clear(){}
    PostGroup mPostGroup;
};

class ReferenceMsgItem: public RsGxsMsgItem
{
public:
    ReferenceMsgItem():
        RsGxsMsgItem(RS_SERVICE_GXS_TYPE_WALLS, RS_PKT_SUBTYPE_WALL_REF_MSG_ITEM) {}
    virtual std::ostream &print(std::ostream &out, uint16_t indent = 0);
    virtual void clear(){}
    ReferenceMsg mReferenceMsg;
};

class WallSerialiser: public RsGxsCommentSerialiser{
public:
    WallSerialiser():
        RsGxsCommentSerialiser(RS_SERVICE_GXS_TYPE_WALLS) {}

    virtual uint32_t size (RsItem *item);
    virtual bool serialise(RsItem *item, void *data, uint32_t *size);
    virtual RsItem *deserialise (void *data, uint32_t *size);

private:

    uint32_t sizeWallGrp(WallGroupItem *item);
    bool serialiseWallGrp(WallGroupItem *item, void *data, uint32_t *size);
    WallGroupItem *deserialiseWallGrp(void *data, uint32_t *size);

    uint32_t sizePostGrp(PostGroupItem *item);
    bool serialisePostGrp(PostGroupItem *item, void *data, uint32_t *size);
    PostGroupItem *deserialisePostGrp(void *data, uint32_t *size);

    uint32_t sizeRefMsg(ReferenceMsgItem *item);
    bool serialiseRefMsg(ReferenceMsgItem *item, void *data, uint32_t *size);
    ReferenceMsgItem *deserialiseRefMsg(void *data, uint32_t *size);

    uint32_t sizePostMsg(PostMsgItem *item);
    bool serialisePostMsg(PostMsgItem *item, void *data, uint32_t *size);
    PostMsgItem *deserialisePostMsg(void *data, uint32_t *size);
};
}//namespace RsWall
