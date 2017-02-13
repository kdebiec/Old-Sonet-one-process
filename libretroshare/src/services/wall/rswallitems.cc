#include "rswallitems.h"

#include <serialiser/rstlvbase.h>
#include <serialiser/rsbaseserial.h>
#include <limits>

namespace RsWall{

uint32_t serialiseByteVectorSize(const std::vector<uint8_t> &vec){
    // can do nothing about overflow here
    // because the returnvalue is just a integer, even in the upper layers
    return vec.size() + 4; // four byte length header
}
bool serialiseByteVector(void* data, uint32_t size, uint32_t* offset, const std::vector<uint8_t> &vec){
    // have to assume: data and offset point to a valid location

    if(vec.size() > std::numeric_limits<uint32_t>::max())
    {
        std::cerr << __func__ << "Error: vector to big for 32 bit size field" << std::endl;
        return false;
    }
    if(*offset > size)
    {
        std::cerr << __func__ << "serious Error: *offset > size" << std::endl;
        return false;
    }
    // calc difference, to avoid overflow if we would calculate a sum
    uint32_t spaceleft = size - *offset;
    uint32_t mysize = serialiseByteVectorSize(vec);
    if(mysize > spaceleft){
        std::cerr << __func__ << "Error: not enaugh space" << std::endl;
        return false;
    }
    bool ok = true;
    ok &= setRawUInt32(data, size, offset, vec.size());
    // can memcpy the vector elements,
    // because a vector stores elements in a contiguous memory area
    if (vec.size() > 0)
    {
        memcpy(&(((uint8_t *) data)[*offset]), &vec[0], vec.size());
        *offset += vec.size();
    }
    return ok;
}
bool deserialiseByteVector(void* data, uint32_t size, uint32_t* offset, std::vector<uint8_t>& vec){
    // assuming data and offset point to a valid location
    // assuming *offset < size
    uint32_t vecsize = 0;
    bool ok = true;
    ok &= getRawUInt32(data, size, offset, &vecsize);
    if(ok && (vecsize > 0)){
        vec.resize(vecsize);
        memcpy(&vec[0], &((uint8_t*)data)[*offset], vecsize);
        *offset += vecsize;
    }else{
        vec.clear();
    }
    return ok;
}

void WallGroupItem::clear()
{
    mProfileText.clear();
    mAvatarImage.mData.clear();
    mWallImage.mData.clear();
}

bool WallGroupItem::fromWallGroup(const WallGroup &group)
{
    mAvatarImage = group.mAvatarImage;
    mWallImage = group.mWallImage;
    mProfileText = group.mProfileText;
    meta = group.mMeta;
}

bool WallGroupItem::toWallGroup(WallGroup &group)
{
    group.mAvatarImage = mAvatarImage;
    group.mWallImage = mWallImage;
    group.mProfileText = mProfileText;
    group.mMeta = meta;
}

void PostMsgItem::clear()
{
    mPostText.clear();
}

bool PostMsgItem::fromWallPost(PostMsg &post)
{
    meta = post.mMeta;
    mPostText = post.mPostText;
}

bool PostMsgItem::toWallPost(PostMsg &post)
{
    post.mMeta = meta;
    post.mPostText = mPostText;
}

std::ostream &WallGroupItem::print(std::ostream &out, uint16_t /*indent*/){
    return out;
}

std::ostream &PostGroupItem::print(std::ostream &out, uint16_t /*indent*/){
    return out;
}

std::ostream &ReferenceMsgItem::print(std::ostream &out, uint16_t /*indent*/){
    return out;
}

std::ostream &PostMsgItem::print(std::ostream &out, uint16_t /*indent*/){
    return out;
}

uint32_t WallSerialiser::size(RsItem *item){

    WallGroupItem *wgi = dynamic_cast<WallGroupItem*>(item);
    if(wgi){
        return sizeWallGrp(wgi);
    }

    PostGroupItem *pgi = dynamic_cast<PostGroupItem*>(item);
    if(pgi){
        return sizePostGrp(pgi);
    }

    ReferenceMsgItem *rmi = dynamic_cast<ReferenceMsgItem*>(item);
    if(rmi){
        return sizeRefMsg(rmi);
    }

    PostMsgItem *pmi = dynamic_cast<PostMsgItem*>(item);
    if(pmi){
        return sizePostMsg(pmi);
    }

    return RsGxsCommentSerialiser::size(item);
}

bool WallSerialiser::serialise(RsItem *item, void *data, uint32_t *size){

    WallGroupItem *wgi = dynamic_cast<WallGroupItem*>(item);
    if(wgi){
        return serialiseWallGrp(wgi, data, size);
    }

    PostGroupItem *pgi = dynamic_cast<PostGroupItem*>(item);
    if(pgi){
        return serialisePostGrp(pgi, data, size);
    }

    ReferenceMsgItem *rmi = dynamic_cast<ReferenceMsgItem*>(item);
    if(rmi){
        return serialiseRefMsg(rmi, data, size);
    }

    PostMsgItem *pmi = dynamic_cast<PostMsgItem*>(item);
    if(pmi){
        return serialisePostMsg(pmi, data, size);
    }

    return RsGxsCommentSerialiser::serialise(item, data, size);
}

RsItem *WallSerialiser::deserialise(void *data, uint32_t *size){

    uint32_t pktId = getRsItemId(data);

    if((getRsItemVersion(pktId) != RS_PKT_VERSION_SERVICE)||(getRsItemService(pktId) != RS_SERVICE_GXS_TYPE_WALLS)){
        return NULL;
    }

    uint8_t pktSubtype = getRsItemSubType(pktId);

    switch(pktSubtype){
    case RS_PKT_SUBTYPE_WALL_WALL_GRP_ITEM:
            return deserialiseWallGrp(data, size);
    case RS_PKT_SUBTYPE_WALL_POST_GRP_ITEM:
            return deserialisePostGrp(data, size);
    case RS_PKT_SUBTYPE_WALL_REF_MSG_ITEM:
            return deserialiseRefMsg(data, size);
    case RS_PKT_SUBTYPE_WALL_POST_MSG_ITEM:
            return deserialisePostMsg(data, size);
    }

    return RsGxsCommentSerialiser::deserialise(data, size);
}

//*************************************************************************************
uint32_t WallSerialiser::sizeWallGrp(WallGroupItem *item){
    uint32_t size = 0;
    size += getRsPktBaseSize();
    size += GetTlvStringSize(item->mProfileText);
    size += serialiseByteVectorSize(item->mAvatarImage.mData);
    size += serialiseByteVectorSize(item->mWallImage.mData);
    return size;
}

bool WallSerialiser::serialiseWallGrp(WallGroupItem *item, void *data, uint32_t *size){
    uint32_t tlvsize = sizeWallGrp(item);
    if(*size < tlvsize){
        return false;
    }
    *size = tlvsize;
    bool ok = true;
    ok &= setRsItemHeader(data, *size, item->PacketId(), tlvsize);
    uint32_t offset = getRsPktBaseSize();
    ok &= SetTlvString(data, *size, &offset, TLV_TYPE_STR_MSG, item->mProfileText);
    ok &= serialiseByteVector(data, *size, &offset, item->mAvatarImage.mData);
    ok &= serialiseByteVector(data, *size, &offset, item->mWallImage.mData);
    return ok;
}

WallGroupItem *WallSerialiser::deserialiseWallGrp(void *data, uint32_t *size){
    WallGroupItem *wgi = new WallGroupItem();
    bool ok = true;
    uint32_t offset = getRsPktBaseSize();;
    ok &= GetTlvString(data, *size, &offset, TLV_TYPE_STR_MSG, wgi->mProfileText);
    ok &= deserialiseByteVector(data, *size, &offset, wgi->mAvatarImage.mData);
    ok &= deserialiseByteVector(data, *size, &offset, wgi->mWallImage.mData);
    if(*size != offset){
        std::cerr << __func__ << "Error: size mismatch" << std::endl;
        ok = false;
    }
    if(ok){
        return wgi;
    }else{
        delete wgi;
        return NULL;
    }
}

//*************************************************************************************
uint32_t WallSerialiser::sizePostGrp(PostGroupItem */*item*/){
    uint32_t size = 0;
    size += getRsPktBaseSize();
    return size;
}

bool WallSerialiser::serialisePostGrp(PostGroupItem *item, void *data, uint32_t *size){

    uint32_t tlvsize = sizePostGrp(item);
    if(*size < tlvsize){
        return false;
    }
    *size = tlvsize;

    bool ok = true;
    ok &= setRsItemHeader(data, *size, item->PacketId(), tlvsize);
    //ok &= ;
    return ok;
}

PostGroupItem *WallSerialiser::deserialisePostGrp(void */*data*/, uint32_t */*size*/){

    PostGroupItem *pgi = new PostGroupItem();
    bool ok = true;

    if(ok){
        return pgi;
    }else{
        delete pgi;
        return NULL;
    }
}

//*************************************************************************************
uint32_t WallSerialiser::sizeRefMsg(ReferenceMsgItem *item){
    uint32_t size = 0;
    size += getRsPktBaseSize();
    size += item->mReferenceMsg.mReferencedGroup.serial_size();
    size += GetTlvUInt32Size(); // reference type
    return size;
}

bool WallSerialiser::serialiseRefMsg(ReferenceMsgItem *item, void *data, uint32_t *size){
    uint32_t tlvsize = sizeRefMsg(item);
    if(*size < tlvsize){
        return false;
    }
    *size = tlvsize;
    bool ok = true;
    ok &= setRsItemHeader(data, *size, item->PacketId(), tlvsize);
    uint32_t offset = getRsPktBaseSize();
    ok &= item->mReferenceMsg.mReferencedGroup.serialise(data, *size, offset);
    ok &= SetTlvUInt32(data, *size, &offset, 0, item->mReferenceMsg.mType);
    //ok &= ;
    return ok;
}

ReferenceMsgItem *WallSerialiser::deserialiseRefMsg(void *data, uint32_t *size){
    ReferenceMsgItem *rfi = new ReferenceMsgItem();
    bool ok = true;
    uint32_t offset = getRsPktBaseSize();
    ok &= rfi->mReferenceMsg.mReferencedGroup.deserialise(data, *size, offset);
    ok &= GetTlvUInt32(data, *size, &offset, 0, &rfi->mReferenceMsg.mType);
    if(*size != offset){
        std::cerr << __func__ << "Error: size mismatch" << std::endl;
        ok = false;
    }
    if(ok){
        return rfi;
    }else{
        delete rfi;
        return NULL;
    }
}

//*************************************************************************************
uint32_t WallSerialiser::sizePostMsg(PostMsgItem *item){
    uint32_t size = 0;
    size += getRsPktBaseSize();
    size += GetTlvStringSize(item->mPostText);
    // todo: other members
    return size;
}

bool WallSerialiser::serialisePostMsg(PostMsgItem *item, void *data, uint32_t *size){
    uint32_t tlvsize = sizePostMsg(item);
    if(*size < tlvsize){
        return false;
    }
    *size = tlvsize;
    bool ok = true;
    ok &= setRsItemHeader(data, *size, item->PacketId(), tlvsize);
    uint32_t offset = getRsPktBaseSize();
    ok &= SetTlvString(data, *size, &offset, TLV_TYPE_STR_MSG, item->mPostText);
    // todo: other members
    //ok &= ;
    return ok;
}

PostMsgItem *WallSerialiser::deserialisePostMsg(void *data, uint32_t *size){
    PostMsgItem *pmi = new PostMsgItem();
    bool ok = true;
    uint32_t offset = getRsPktBaseSize();
    ok &= GetTlvString(data, *size, &offset, TLV_TYPE_STR_MSG, pmi->mPostText);
    if(ok){
        return pmi;
    }else{
        delete pmi;
        return NULL;
    }
}

}//namespace RsWall
