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
#ifndef OPUSPROCESSOR_H
#define OPUSPROCESSOR_H

//Qt
#include <QObject>
#include <QIODevice>

//Opus
#include <opus/opus.h>

class QtOpusInputProcessor: public QIODevice
{
    Q_OBJECT
public:
    QtOpusInputProcessor(QObject* parent = 0);
    ~QtOpusInputProcessor();

    bool hasPendingPackets();
    QByteArray getNetworkPacket();

signals:
    void networkPacketReady();

protected:
    virtual qint64 readData(char * /*data*/, qint64 /*maxSize*/) {return false;} //not used for input processor
    virtual qint64 writeData(const char *data, qint64 maxSize);

private:
    OpusEncoder *encoder;
    typedef std::array<unsigned char, 3*1276> EncodingOutputBuffer;
    QByteArray inputBuffer;
    QList<QByteArray> outputNetworkBuffer;
};

class QtOpusOutputProcessor: public QIODevice
{
    Q_OBJECT
public:
    QtOpusOutputProcessor(QObject* parent = 0);
    ~QtOpusOutputProcessor();

    void putNetworkPacket(QString name, QByteArray packet);

protected:
    virtual qint64 readData(char *data, qint64 maxSize);
    virtual qint64 writeData(const char * /*data*/, qint64 /*maxSize*/) {return 0;} //not used for output processor

private:
    OpusDecoder *decoder;
    QByteArray outputBuffer;
    QList<QByteArray> inputNetworkBuffer;
};
#endif // OPUSPROCESSOR_H
