/****************************************************************
 *  This file is part of Sonet.
 *  Sonet is distributed under the following license:
 *
 *  Copyright 2012-2012 by Robert Fernie.
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
#include "util/TokenQueue.h"

//std
#include <iostream>

//Qt
#include <QTimer>

/** Constructor */
TokenQueue::TokenQueue(RsTokenService *service, TokenResponse *resp)
	: QObject(NULL), mService(service), mResponder(resp)
{
    mTrigger = new QTimer ;
    mTrigger->setInterval(499);
    mTrigger->setSingleShot(true);
    connect(mTrigger, SIGNAL(timeout()), this, SLOT(pollRequests()));
}

bool TokenQueue::requestGroupInfo(uint32_t &token, uint32_t anstype, const RsTokReqOptions &opts, std::list<RsGxsGroupId>& ids, uint32_t usertype)
{
	uint32_t basictype = TOKENREQ_GROUPINFO;
	mService->requestGroupInfo(token, anstype, opts, ids);
	queueRequest(token, basictype, anstype, usertype);

	return true;
}

bool TokenQueue::requestGroupInfo(uint32_t &token, uint32_t anstype, const RsTokReqOptions &opts, uint32_t usertype)
{
	uint32_t basictype = TOKENREQ_GROUPINFO;
	mService->requestGroupInfo(token, anstype, opts);
	queueRequest(token, basictype, anstype, usertype);

	return true;
}

bool TokenQueue::requestMsgInfo(uint32_t &token, uint32_t anstype, const RsTokReqOptions &opts, const GxsMsgReq& ids, uint32_t usertype)
{
	uint32_t basictype = TOKENREQ_MSGINFO;
	mService->requestMsgInfo(token, anstype, opts, ids);
	queueRequest(token, basictype, anstype, usertype);

	return true;
}

bool TokenQueue::requestMsgRelatedInfo(uint32_t &token, uint32_t anstype,  const RsTokReqOptions &opts, const std::vector<RsGxsGrpMsgIdPair> &msgId, uint32_t usertype)
{
	uint32_t basictype = TOKENREQ_MSGINFO;
	mService->requestMsgRelatedInfo(token, anstype, opts, msgId);
	queueRequest(token, basictype, anstype, usertype);

	return true;
}

bool TokenQueue::requestMsgInfo(uint32_t &token, uint32_t anstype, const RsTokReqOptions &opts, const std::list<RsGxsGroupId> &grpIds, uint32_t usertype)
{
	uint32_t basictype = TOKENREQ_MSGINFO;
	mService->requestMsgInfo(token, anstype, opts, grpIds);
	queueRequest(token, basictype, anstype, usertype);

	return true;
}

void TokenQueue::queueRequest(uint32_t token, uint32_t basictype, uint32_t anstype, uint32_t usertype)
{
#ifdef ID_DEBUG
	std::cerr << "TokenQueue::queueRequest() Token: " << token << " Type: " << basictype;
	std::cerr << " AnsType: " << anstype << " UserType: " << usertype;
    std::cerr << std::endl;
#endif

	TokenRequest req;
	req.mToken = token;
	req.mType = basictype;
	req.mAnsType = anstype;
	req.mUserType = usertype;

	gettimeofday(&req.mRequestTs, NULL);
	req.mPollTs = req.mRequestTs;

	mRequests.push_back(req);

	if (mRequests.size() == 1)
	{
		/* start the timer */
		doPoll(0.25);
	}
}

void TokenQueue::doPoll(float dt)
{
	/* single shot poll */
	mTrigger->start(dt * 1000);
}

void TokenQueue::pollRequests()
{
	double pollPeriod = 0.5; // max poll period.

    if (mRequests.empty())
        return;

	TokenRequest req;

	req = mRequests.front();
	mRequests.pop_front();

	if (checkForRequest(req.mToken))
	{
		/* clean it up and handle */
		loadRequest(req);
	}
	else
        mRequests.push_back(req);

	if (mRequests.size() > 0)
		doPoll(pollPeriod);
}

bool TokenQueue::checkForRequest(uint32_t token)
{
	/* check token */
	uint32_t status =  mService->requestStatus(token);
	return ( (RsTokenService::GXS_REQUEST_V2_STATUS_FAILED == status) ||
			 (RsTokenService::GXS_REQUEST_V2_STATUS_COMPLETE == status) );
}

bool TokenQueue::activeRequestExist(const uint32_t& userType) const
{
	std::list<TokenRequest>::const_iterator lit = mRequests.begin();

	for(; lit != mRequests.end(); ++lit)
	{
		const TokenRequest& req = *lit;

        if(req.mUserType == userType)
            return true;
	}

	return false;
}

void TokenQueue::activeRequestTokens(const uint32_t& userType, std::list<uint32_t>& tokens) const
{
	std::list<TokenRequest>::const_iterator lit = mRequests.begin();

	for(; lit != mRequests.end(); ++lit)
	{
		const TokenRequest& req = *lit;

		if(req.mUserType == userType)
			tokens.push_back(req.mToken);
	}
}

void TokenQueue::cancelActiveRequestTokens(const uint32_t& userType)
{
	std::list<uint32_t> tokens;
	activeRequestTokens(userType, tokens);
    if (!tokens.empty())
    {
		std::list<uint32_t>::iterator tokenIt;
        for (tokenIt = tokens.begin(); tokenIt != tokens.end(); ++tokenIt)
            cancelRequest(*tokenIt);
	}
}

void TokenQueue::loadRequest(const TokenRequest &req)
{
#ifdef DEBUG_INFO
    std::cerr << "TokenQueue::loadRequest(): ";
	std::cerr << "Token: " << req.mToken << " Type: " << req.mType;
	std::cerr << " AnsType: " << req.mAnsType << " UserType: " << req.mUserType;
    std::cerr << std::endl;
#endif

	mResponder->loadRequest(this, req);
}

bool TokenQueue::cancelRequest(const uint32_t token)
{
	/* cancel at lower level first */
	mService->cancelRequest(token);

	std::list<TokenRequest>::iterator it;

	for(it = mRequests.begin(); it != mRequests.end(); ++it)
	{
		if (it->mToken == token)
		{
            mRequests.erase(it);

#ifdef DEBUG_INFO
            std::cerr << "TokenQueue::cancelRequest() Cleared Request: " << token;
            std::cerr << std::endl;
#endif

			return true;
		}
	}

	std::cerr << "TokenQueue::cancelRequest() Failed to Find Request: " << token;
	std::cerr << std::endl;

	return false;
}
