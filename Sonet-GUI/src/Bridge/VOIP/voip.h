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
#ifndef VOIP_H
#define VOIP_H

//Qt
#include <QObject>
#include <QTimer>
#include <QSound>

//libretroshare
#include <retroshare/rsidentity.h>

//Sonet-GUI
#include "Bridge/VOIP/opusprocessor.h"



class QAudioInput;
class QAudioOutput;

class p3NotifyVOIP;

class VOIP : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString gxs MEMBER gxs NOTIFY gxsChanged)                // Actually, only for calling
    Q_PROPERTY(QString rspgpid MEMBER rspgpid NOTIFY rspgpChanged)      // Actually, only for calling
    Q_PROPERTY(QString rspeerid MEMBER rspeerid NOTIFY rspeerChanged)    // Actually, only for answering

public:
    VOIP(QObject *parent = 0); // In order to create objects in QML
    VOIP(QString &rs, QObject *parent = 0);
    VOIP(RsPgpId &rs, QObject *parent = 0);
    VOIP(const RsPeerId &rs, QObject *parent = 0);
        virtual~VOIP();

    void addAudioData(const RsPeerId &peer_id, QByteArray* array);
    RsPeerId getRsPeerId();

signals:
    void gxsChanged();
    void rspgpChanged();
    void rspeerChanged();

    void acceptReceived();
    void hangupReceived();
    void acceptSended();

public slots:
    void sendAudioData();
    void startAudioCapture(); //AnswerAudioCall
    void hangupCall() ;

    void muteMic();
    void unmuteMic();

    void muteReceiver();
    void unmuteReceiver();

    void setOutputVolume(qreal volume);

    void receivedVoipData(const RsPeerId &peer_id);
    void receivedVoipHangUp(const RsPeerId &peer_id, int flags);
    void receivedVoipAccept(const RsPeerId &peer_id, int flags);

private slots:
    void timerAudioRingTimeOut();

    void gxsSlot();
    void rspgpSlot();
    void rspeerSlot();

private:
    // Audio input/output
    QAudioInput* inputAudioDevice;
    QAudioOutput* outputAudioDevice;

    QtOpusInputProcessor* inputAudioProcessor;
    QtOpusOutputProcessor* outputAudioProcessor;

    //Waiting for peer accept
    QTimer *timerAudioRing;
    int sendAudioRingTime; //(-2 connected, -1 reseted, >=0 in progress)
    int recAudioRingTime; //(-2 connected, -1 reseted, >=0 in progress)

    bool isMuted;
    bool isMutedReceiver;
    RsPeerId rsPeer;
    p3NotifyVOIP *mNotify;

    QString gxs;
    QString rspgpid;
    QString rspeerid;

    QSound *qsound;

    qreal outputVolume;
};

#endif // VOIP_H
