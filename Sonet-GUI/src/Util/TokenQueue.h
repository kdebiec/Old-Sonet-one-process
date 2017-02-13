/****************************************************************
 *  This file is part of Sonet.
 *  Sonet is distributed under the following license:
 *
 *  Copyright 2012-2012 by Robert Fernie, Christopher Evi-Parker
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

#ifndef MRK_TOKEN_QUEUE_V2_H
#define MRK_TOKEN_QUEUE_V2_H

//std
#include <list>
#include <string>
#include <sys/time.h>

//Qt
#include <QWidget>
#include <QTimer>

//libretroshare
#include <retroshare/rstokenservice.h>


#define COMPLETED_REQUEST       4

#define TOKENREQ_GROUPINFO      1
#define TOKENREQ_MSGINFO        2
#define TOKENREQ_MSGRELATEDINFO 3

class TokenQueue;

class TokenRequest
{
public:
	uint32_t mToken;
	uint32_t mType;
	uint32_t mAnsType;
	uint32_t mUserType;
	struct timeval mRequestTs;
	struct timeval mPollTs;
};

class TokenResponse
{
public:
	// These Functions are overloaded to get results out.
	virtual void loadRequest(const TokenQueue *queue, const TokenRequest &req) = 0;
};


/*!
 * An important thing to note is that all requests are stacked (so FIFO)
 * This is to prevent overlapped loads on GXS UIs
 */
class TokenQueue: public QObject
{
	Q_OBJECT

public:
	TokenQueue(RsTokenService *service, TokenResponse *resp);

	/* generic handling of token / response update behaviour */

	/*!
	 *
	 * @token the token to be redeem is assigned here
	 *
	 */
	bool requestGroupInfo(uint32_t &token, uint32_t anstype, const RsTokReqOptions &opts,
						  std::list<RsGxsGroupId>& ids, uint32_t usertype);

	bool requestGroupInfo(uint32_t &token, uint32_t anstype, const RsTokReqOptions &opts, uint32_t usertype);

	bool requestMsgInfo(uint32_t &token, uint32_t anstype, const RsTokReqOptions &opts,
						const std::list<RsGxsGroupId>& grpIds, uint32_t usertype);

	bool requestMsgInfo(uint32_t &token, uint32_t anstype, const RsTokReqOptions &opts,
						const GxsMsgReq& grpIds, uint32_t usertype);

	bool requestMsgRelatedInfo(uint32_t &token, uint32_t anstype,  const RsTokReqOptions &opts, const std::vector<RsGxsGrpMsgIdPair>& msgId, uint32_t usertype);

	bool cancelRequest(const uint32_t token);

	void queueRequest(uint32_t token, uint32_t basictype, uint32_t anstype, uint32_t usertype);
	bool checkForRequest(uint32_t token);
	void loadRequest(const TokenRequest &req);

	bool activeRequestExist(const uint32_t& userType) const;
	void activeRequestTokens(const uint32_t& userType, std::list<uint32_t>& tokens) const;
	void cancelActiveRequestTokens(const uint32_t& userType);

protected:
	void doPoll(float dt);

private slots:
	void pollRequests();

private:
	/* Info for Data Requests */
	std::list<TokenRequest> mRequests;

	RsTokenService *mService;
	TokenResponse *mResponder;

	QTimer *mTrigger;
};

#endif
