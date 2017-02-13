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
#include "voip.h"

//STD
#include <time.h>

//libretroshare
#include <rsserver/p3face.h>
#include <retroshare/rsstatus.h>
#include <retroshare/rspeers.h>
#include <services/VOIP/rsVOIP.h>
#include <services/VOIP/rsVOIPItems.h>
#include <services/VOIP/p3voipservice.h>

//Sonet-GUI
#include "Bridge/VOIP/audiodevicehelper.h"
#include "Bridge/VOIP/vnotify.h"
#include "Bridge/SoundManager.h"

VOIP::VOIP(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<RsPeerId>("RsPeerId");

    mNotify = RsServer::notifyVOIP();

    outputAudioProcessor = NULL ;
    outputAudioDevice = NULL ;
    inputAudioProcessor = NULL ;
    inputAudioDevice = NULL ;

    sendAudioRingTime = -1;
    recAudioRingTime = -1;

    outputVolume = 1;

    qsound = soundManager->playEvent(SOUND_CALLING);
    if(qsound->fileName() != "")
        qsound->setLoops(QSound::Infinite);

    isMuted = true;
    isMutedReceiver = false;

    QObject::connect(this,  SIGNAL(gxsChanged()),   this,   SLOT(gxsSlot()));
    QObject::connect(this,  SIGNAL(rspgpChanged()), this,   SLOT(rspgpSlot()));
    QObject::connect(this,  SIGNAL(rspeerChanged()),this,   SLOT(rspeerSlot()));

}

VOIP::VOIP(QString &rs, QObject *parent) : QObject(parent)
{
    mNotify = RsServer::notifyVOIP();

    RsGxsId gxsId(rs.toStdString());
    RsIdentityDetails details;

    rsIdentity->getIdDetails(gxsId, details);
    if (details.mPgpId.isNull())
    {
        while(details.mPgpId.isNull())
        {
            rsIdentity->getIdDetails(gxsId, details);
            Sleep(1000);
        }
    }

    RsPgpId entry;
    entry = details.mPgpId;

    RsPeerDetails detail;

    rsPeers->getGPGDetails(entry, detail);

    //let's get the ssl child details
    std::list<RsPeerId> sslIds;
    rsPeers->getAssociatedSSLIds(detail.gpg_id, sslIds);

    bool oneid = true;
    if (sslIds.size() == 1)
    {
        rsPeer = sslIds.front();
        oneid = false;
    }

    if(oneid)
    {
        // more than one ssl ids available, check for online
        std::list<RsPeerId> onlineIds;
        for (std::list<RsPeerId>::iterator it = sslIds.begin(); it != sslIds.end(); ++it)
        {
            if (rsPeers->isOnline(*it))
                onlineIds.push_back(*it);
        }

        if (onlineIds.size() == 1)
            rsPeer = onlineIds.front();
    }

    outputAudioProcessor = NULL ;
    outputAudioDevice = NULL ;
    inputAudioProcessor = NULL ;
    inputAudioDevice = NULL ;

    sendAudioRingTime = -1;
    recAudioRingTime = -1;

    outputVolume = 1;

    timerAudioRing = new QTimer(this);
    timerAudioRing->setInterval(1000);
    timerAudioRing->setSingleShot(true);
    connect(timerAudioRing, SIGNAL(timeout()), this, SLOT(timerAudioRingTimeOut()));

    qsound = soundManager->playEvent(SOUND_CALLING);
    if(qsound->fileName() != "")
        qsound->setLoops(QSound::Infinite);

    isMuted = true;
    isMutedReceiver = false;
}

VOIP::VOIP(RsPgpId &rs, QObject *parent) : QObject(parent)
{
    RsPgpId entry;
    entry = rs;

    RsPeerDetails detail;
    rsPeers->getGPGDetails(entry, detail);

    //let's get the ssl child details
    std::list<RsPeerId> sslIds;
    rsPeers->getAssociatedSSLIds(detail.gpg_id, sslIds);

    bool oneid = true;
    if (sslIds.size() == 1)
    {
        rsPeer = sslIds.front();
        oneid = false;
    }

    if(oneid)
    {
        // more than one ssl ids available, check for online
        std::list<RsPeerId> onlineIds;
        for (std::list<RsPeerId>::iterator it = sslIds.begin(); it != sslIds.end(); ++it)
        {
            if (rsPeers->isOnline(*it))
                onlineIds.push_back(*it);
        }

        if (onlineIds.size() == 1)
            rsPeer = onlineIds.front();
    }

    outputAudioProcessor = NULL ;
    outputAudioDevice = NULL ;
    inputAudioProcessor = NULL ;
    inputAudioDevice = NULL ;

    sendAudioRingTime = -1;
    recAudioRingTime = -1;

    outputVolume = 1;

    timerAudioRing = new QTimer(this);
    timerAudioRing->setInterval(1000);
    timerAudioRing->setSingleShot(true);
    connect(timerAudioRing, SIGNAL(timeout()), this, SLOT(timerAudioRingTimeOut()));

    qsound = soundManager->playEvent(SOUND_CALLING);
    if(qsound->fileName() != "")
        qsound->setLoops(QSound::Infinite);

    isMuted = true;
    isMutedReceiver = false;
}

VOIP::VOIP(const RsPeerId &rs, QObject *parent) : QObject(parent), rsPeer(rs)
{
    mNotify = RsServer::notifyVOIP();

    outputAudioProcessor = NULL ;
    outputAudioDevice = NULL ;
    inputAudioProcessor = NULL ;
    inputAudioDevice = NULL ;

    sendAudioRingTime = -1;
    recAudioRingTime = 0;

    outputVolume = 1;

    timerAudioRing = new QTimer(this);
    timerAudioRing->setInterval(1000);
    timerAudioRing->setSingleShot(true);
    connect(timerAudioRing, SIGNAL(timeout()), this, SLOT(timerAudioRingTimeOut()));

    qsound = soundManager->playEvent(SOUND_CALLING);
    if(qsound->fileName() != "")
        qsound->setLoops(QSound::Infinite);

    isMuted = true;
    isMutedReceiver = false;
}

VOIP::~VOIP()
{
    if(qsound != NULL)
        qsound->stop();

    hangupCall();

    if(inputAudioDevice != NULL)
        inputAudioDevice->stop();

    // //////////////////////////////////////////

    if(outputAudioDevice != NULL)
        outputAudioDevice->stop();

    // //////////////////////////////////////////

    // stop and delete timers
    timerAudioRing->stop();
    if(timerAudioRing != NULL)
        delete timerAudioRing;

    timerAudioRing = NULL;

    // //////////////////////////////////////////

    if(inputAudioProcessor != NULL)
        delete inputAudioProcessor;
    inputAudioProcessor = NULL;

    // //////////////////////////////////////////

    if(outputAudioProcessor != NULL)
        delete outputAudioProcessor;
    outputAudioProcessor = NULL;
}

void VOIP::gxsSlot()
{
    if(!gxs.isEmpty())
    {
        RsGxsId gxsId(gxs.toStdString());
        RsIdentityDetails details;

        rsIdentity->getIdDetails(gxsId, details);
        if (details.mPgpId.isNull())
        {
            while(details.mPgpId.isNull())
            {
                rsIdentity->getIdDetails(gxsId, details);
                Sleep(1000);
            }
        }

        RsPgpId entry = details.mPgpId;

        RsPeerDetails detail;
        rsPeers->getGPGDetails(entry, detail);

        //let's get the ssl child details
        std::list<RsPeerId> sslIds;
        rsPeers->getAssociatedSSLIds(detail.gpg_id, sslIds);

        bool oneid = true;
        if (sslIds.size() == 1)
        {
            rsPeer = sslIds.front();
            oneid = false;
        }

        if(oneid)
        {
            // more than one ssl ids available, check for online
            std::list<RsPeerId> onlineIds;
            for (std::list<RsPeerId>::iterator it = sslIds.begin(); it != sslIds.end(); ++it)
            {
                if (rsPeers->isOnline(*it))
                    onlineIds.push_back(*it);
            }

            if (onlineIds.size() == 1)
                rsPeer = onlineIds.front();
        }

        QObject::connect(VNotify::getInstance(), SIGNAL(voipDataReceived(const RsPeerId&)),            this,   SLOT(receivedVoipData(const RsPeerId&)),            Qt::QueuedConnection);
        QObject::connect(VNotify::getInstance(), SIGNAL(voipAcceptReceived(const RsPeerId&,int)),      this,   SLOT(receivedVoipAccept(const RsPeerId&,int)),      Qt::QueuedConnection);
        QObject::connect(VNotify::getInstance(), SIGNAL(voipHangUpReceived(const RsPeerId&,int)),      this,   SLOT(receivedVoipHangUp(const RsPeerId&,int)),      Qt::QueuedConnection);

        timerAudioRing = new QTimer(this);
        timerAudioRing->setInterval(1000);
        timerAudioRing->setSingleShot(true);
        connect(timerAudioRing, SIGNAL(timeout()), this, SLOT(timerAudioRingTimeOut()));

        startAudioCapture();
    }
    else
        std::cerr << "ERROR VOIP::gxsSlot() private member is empty" << std::endl;

}

void VOIP::rspgpSlot()
{
    RsPgpId entry(rspgpid.toStdString());

    RsPeerDetails detail;

    rsPeers->getGPGDetails(entry, detail);

    //let's get the ssl child details
    std::list<RsPeerId> sslIds;
    rsPeers->getAssociatedSSLIds(detail.gpg_id, sslIds);

    bool oneid = true;
    if (sslIds.size() == 1)
    {
        rsPeer = sslIds.front();
        oneid = false;
    }

    if(oneid)
    {
        // more than one ssl ids available, check for online
        std::list<RsPeerId> onlineIds;
        for (std::list<RsPeerId>::iterator it = sslIds.begin(); it != sslIds.end(); ++it)
        {
            if (rsPeers->isOnline(*it))
                onlineIds.push_back(*it);
        }

        if (onlineIds.size() == 1)
            rsPeer = onlineIds.front();
    }

    QObject::connect(VNotify::getInstance(), SIGNAL(voipDataReceived(const RsPeerId&)),            this,   SLOT(receivedVoipData(const RsPeerId&)),            Qt::QueuedConnection);
    QObject::connect(VNotify::getInstance(), SIGNAL(voipAcceptReceived(const RsPeerId&,int)),      this,   SLOT(receivedVoipAccept(const RsPeerId&,int)),      Qt::QueuedConnection);
    QObject::connect(VNotify::getInstance(), SIGNAL(voipHangUpReceived(const RsPeerId&,int)),      this,   SLOT(receivedVoipHangUp(const RsPeerId&,int)),      Qt::QueuedConnection);

    timerAudioRing = new QTimer(this);
    timerAudioRing->setInterval(1000);
    timerAudioRing->setSingleShot(true);
    connect(timerAudioRing, SIGNAL(timeout()), this, SLOT(timerAudioRingTimeOut()));

    startAudioCapture();
}

void VOIP::rspeerSlot()
{
    recAudioRingTime = 0;
    rsPeer = RsPeerId(rspeerid.toStdString());

    QObject::connect(VNotify::getInstance(), SIGNAL(voipDataReceived(const RsPeerId&)),            this,   SLOT(receivedVoipData(const RsPeerId&)),            Qt::QueuedConnection);
    QObject::connect(VNotify::getInstance(), SIGNAL(voipAcceptReceived(const RsPeerId&,int)),      this,   SLOT(receivedVoipAccept(const RsPeerId&,int)),      Qt::QueuedConnection);
    QObject::connect(VNotify::getInstance(), SIGNAL(voipHangUpReceived(const RsPeerId&,int)),      this,   SLOT(receivedVoipHangUp(const RsPeerId&,int)),      Qt::QueuedConnection);

    timerAudioRing = new QTimer(this);
    timerAudioRing->setInterval(1000);
    timerAudioRing->setSingleShot(true);
    connect(timerAudioRing, SIGNAL(timeout()), this, SLOT(timerAudioRingTimeOut()));

    startAudioCapture();
}

RsPeerId VOIP::getRsPeerId()
{
    return rsPeer;
}

void VOIP::hangupCall()
{

    rsVOIP->sendVoipHangUpCall(rsPeer, RS_VOIP_FLAGS_AUDIO_DATA);
    qsound->stop();
    sendAudioRingTime = -1;
    recAudioRingTime = -1;
}

void VOIP::startAudioCapture()
{
    isMuted = false;
    if (recAudioRingTime == -1 && sendAudioRingTime == -1)
    {
        sendAudioRingTime = 0;
        rsVOIP->sendVoipRinging(rsPeer, RS_VOIP_FLAGS_AUDIO_DATA);
        timerAudioRingTimeOut();
        if(!isMutedReceiver)
            qsound->play();
        return; //Start Audio when accept received
    }
    if (recAudioRingTime != -1)
    {
        rsVOIP->sendVoipAcceptCall(rsPeer, RS_VOIP_FLAGS_AUDIO_DATA);
        emit acceptSended();
    }
    recAudioRingTime = -1; //Stop ringing
    qsound->stop();

    //activate audio input
    if (!inputAudioProcessor)
    {
        inputAudioProcessor = new QtOpusInputProcessor();
        inputAudioProcessor->open(QIODevice::WriteOnly | QIODevice::Unbuffered);
    }
    if (!inputAudioDevice)
    {
       inputAudioDevice = AudioDeviceHelper::getPreferedInputDevice();
    }
    connect(inputAudioProcessor, SIGNAL(networkPacketReady()), this, SLOT(sendAudioData()));
    inputAudioDevice->start(inputAudioProcessor);
}

void VOIP::muteMic()
{
    if (!isMuted)
    {
        disconnect(inputAudioProcessor, SIGNAL(networkPacketReady()), this, SLOT(sendAudioData()));
        if (inputAudioDevice)
            inputAudioDevice->stop();

        isMuted = true;
    }
}

void VOIP::unmuteMic()
{
    if (isMuted)
    {
        connect(inputAudioProcessor, SIGNAL(networkPacketReady()), this, SLOT(sendAudioData()));
        if (inputAudioDevice)
            inputAudioDevice->start(inputAudioProcessor);

        isMuted = false;
    }
}

void VOIP::muteReceiver()
{
    if(!isMutedReceiver)
    {
        if (sendAudioRingTime >= 0)
            qsound->stop();

        isMutedReceiver = true;
    }
}

void VOIP::unmuteReceiver()
{
    if(isMutedReceiver)
    {
        if (sendAudioRingTime >= 0)
            qsound->play();

        isMutedReceiver = false;
    }
}

void VOIP::addAudioData(const RsPeerId &peer_id, QByteArray* array)
{
    if(peer_id == rsPeer)
    {
        sendAudioRingTime = -2;//Receive Audio so Accepted

        if(!isMutedReceiver)
        {
            if (!outputAudioDevice)
                outputAudioDevice = AudioDeviceHelper::getDefaultOutputDevice();

            if (!outputAudioProcessor)
            {
                //start output audio device
                outputAudioProcessor = new QtOpusOutputProcessor();
                outputAudioProcessor->open(QIODevice::ReadOnly | QIODevice::Unbuffered);
                outputAudioDevice->start(outputAudioProcessor);
            }

            if (outputAudioDevice && outputAudioDevice->error() != QAudio::NoError)
            {
                std::cerr << "Restarting output device. Error before reset " << outputAudioDevice->error() << " buffer size : " << outputAudioDevice->bufferSize() << std::endl;
                outputAudioDevice->stop();
                outputAudioDevice->reset();
                if (outputAudioDevice->error() == QAudio::UnderrunError)
                    outputAudioDevice->setBufferSize(20);
                outputAudioDevice->start(outputAudioProcessor);
            }
            outputAudioDevice->setVolume(outputVolume);
            outputAudioProcessor->putNetworkPacket(QString::fromStdString(peer_id.toStdString()), *array);
        }
        //check the input device for errors
        if (inputAudioDevice && inputAudioDevice->error() != QAudio::NoError)
        {
            std::cerr << "Restarting input device. Error before reset " << inputAudioDevice->error() << std::endl;
            inputAudioDevice->stop();
            inputAudioDevice->reset();
            inputAudioDevice->start(inputAudioProcessor);
        }
    }
}

void VOIP::sendAudioData()
{
    while(inputAudioProcessor && inputAudioProcessor->hasPendingPackets())
    {
        QByteArray qbarray = inputAudioProcessor->getNetworkPacket();
        RsVOIPDataChunk chunk;
        chunk.size = qbarray.size();
        chunk.data = (void*)qbarray.constData();
        chunk.type = RsVOIPDataChunk::RS_VOIP_DATA_TYPE_AUDIO ;
        rsVOIP->sendVoipData(rsPeer,chunk);
    }
}

void VOIP::receivedVoipData(const RsPeerId &peer_id)
{
    if(peer_id == rsPeer)
    {
        std::vector<RsVOIPDataChunk> chunks ;

        if(!rsVOIP->getIncomingData(peer_id,chunks))
        {
            std::cerr << "VOIP::receivedVoipData(): No data chunks to get. Weird!" << std::endl;
            return ;
        }
        for (unsigned int chunkIndex=0; chunkIndex<chunks.size(); chunkIndex++)
        {
            QByteArray qb(reinterpret_cast<const char *>(chunks[chunkIndex].data),chunks[chunkIndex].size);

            if(chunks[chunkIndex].type == RsVOIPDataChunk::RS_VOIP_DATA_TYPE_AUDIO)
                addAudioData(peer_id, &qb);
            else if(chunks[chunkIndex].type == RsVOIPDataChunk::RS_VOIP_DATA_TYPE_VIDEO)
                std::cerr << "VOIP::receivedVoipData(): RsVOIPDataChunk::RS_VOIP_DATA_TYPE_VIDEO" << std::endl;
            else
                std::cerr << "VOIPGUIHandler::ReceivedVoipData(): Unknown data type received. type=" << chunks[chunkIndex].type << std::endl;
        }
    }
}

void VOIP::receivedVoipHangUp(const RsPeerId &peer_id, int flags)
{
    if(peer_id == rsPeer)
    {
        hangupCall();
        emit hangupReceived();
    }
}

void VOIP::receivedVoipAccept(const RsPeerId &peer_id, int flags)
{
    if(peer_id == rsPeer)
    {
        sendAudioRingTime = -2;
        emit acceptReceived();
        startAudioCapture();
    }
}

void VOIP::timerAudioRingTimeOut()
{
    // Endless ringing

    //Sending or receiving (-2 connected, -1 reseted, >=0 in progress)
    if (sendAudioRingTime >= 0)
    {
        //Sending
        ++sendAudioRingTime;
        timerAudioRing->start();
    }
    else if(recAudioRingTime >= 0)
    {
        //Receiving
        ++recAudioRingTime;
        if (recAudioRingTime == 100)
            recAudioRingTime = 0;

        if (mNotify)
            mNotify->notifyReceivedVoipAudioCall(rsPeer);

        timerAudioRing->start();
    }
}

void VOIP::setOutputVolume(qreal volume)
{
    outputVolume = (volume/100);
}
