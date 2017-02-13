/****************************************************************
 *  This file is part of Sonet.
 *  Sonet is distributed under the following license:
 *
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
#include "voiphandler.h"

//Qt
#include <QQmlContext>

//libretroshare
#include <retroshare/rspeers.h>

//Sonet-GUI
#include "Bridge/VOIP/vnotify.h"
#include "sonetsettings.h"
#include "Bridge/PopUp/popup.h"
#include "util/imageprovider.h"

/*static*/ VOIPHandler *VOIPHandler::_instance = NULL;

/*static*/ VOIPHandler *VOIPHandler::Create()
{
    if (_instance == NULL)
        _instance = new VOIPHandler();

    return _instance;
}

/*static*/ void VOIPHandler::Destroy()
{
    if(_instance != NULL)
        delete _instance ;

    _instance = NULL ;
}

/*static*/ VOIPHandler *VOIPHandler::getInstance()
{
    return _instance;
}

VOIPHandler::VOIPHandler(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<RsPeerId>("RsPeerId");
    QObject::connect(VNotify::getInstance(), SIGNAL(voipInvitationReceived(const RsPeerId&,int)),
                     this,   SLOT(invitationReceivedSlot(const RsPeerId&,int)),   Qt::QueuedConnection);
}

void VOIPHandler::invitationReceivedSlot(const RsPeerId &peer_id, int /*flags*/)
{
    QString friendname;
    RsPeerDetails detail;
    rsPeers->getPeerDetails(peer_id, detail);
    if(!Settings->getAdvancedMode())
    {
        RsTokReqOptions opts;
        opts.mReqType = GXS_REQUEST_TYPE_GROUP_DATA;

        uint32_t token;

        rsIdentity->getTokenService()->requestGroupInfo(token, RS_TOKREQ_ANSTYPE_DATA, opts);

        if(!waitForTokenOrTimeout(token, rsIdentity->getTokenService()))
            return;

        std::vector<RsGxsIdGroup> datavector;
        if (!rsIdentity->getGroupData(token, datavector))
        {
            std::cerr << "GxsIdModel::updateData() Error getting GroupData" << std::endl;
            return;
        }

        for(std::vector<RsGxsIdGroup>::iterator gidIt = datavector.begin(); gidIt != datavector.end(); ++gidIt)
        {
            if(gidIt->mPgpId == detail.gpg_id)
                friendname = QString::fromStdString(gidIt->mMeta.mGroupName);
        }
    }
    else
        friendname = QString::fromStdString(detail.name);

    PopUp *popUp = new PopUp();
    popUp->setPeerId(QString::fromStdString(peer_id.toStdString()));
    QObject::connect(VNotify::getInstance(), SIGNAL(voipHangUpReceived(const RsPeerId&,int)), popUp, SLOT(hangupReceived(RsPeerId, int)));
    popUp->engine()->addImageProvider(QLatin1String("avatar"), ImageProvider::getInstance());
    popUp->rootContext()->setContextProperty("friendname", friendname);
    popUp->rootContext()->setContextProperty("peerid", QString::fromStdString(peer_id.toStdString()));
    popUp->rootContext()->setContextProperty("voip", this);
    popUp->setSource(QUrl("qrc:/VoipInvitation.qml"));
    popUp->show();

}

void VOIPHandler::accept(QString peerId, QString friendname)
{
    std::cerr << "VOIPHandler::accept()" << std::endl;
    emit voipConnection(peerId, friendname);
}

void VOIPHandler::hangup(QString peerId)
{
    std::cerr << "VOIPHandler::hangup()" << std::endl;

    VOIP *pVoip = new VOIP(RsPeerId(peerId.toStdString()));
    if(pVoip != NULL)
    {
        pVoip->hangupCall();
        delete pVoip;
    }
    pVoip = NULL;
}
