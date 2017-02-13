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
#include "opusprocessor.h"

//std
#include <iostream>

//Qt
#include <QAudio>
#include <QByteArray>

//Sonet-GUI
#include "audiodevicehelper.h"

#define FRAME_SIZE 480//SAMPLE_RATE/100
#define SAMPLE_RATE 48000
#define CHANNELS 1//2
#define APPLICATION OPUS_APPLICATION_VOIP
#define BITRATE 64000

#define MAX_FRAME_SIZE 6*480
#define MAX_PACKET_SIZE (3*1276)

QtOpusInputProcessor::QtOpusInputProcessor(QObject *parent) : QIODevice(parent)
{
    int err;
    encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, APPLICATION, &err);
    if (err<0)
       std::cerr << stderr << "failed to create an encoder: %s\n" << opus_strerror(err) << std::endl;
}

QtOpusInputProcessor::~QtOpusInputProcessor()
{
    opus_encoder_destroy(encoder);
}

bool QtOpusInputProcessor::hasPendingPackets()
{
    return !outputNetworkBuffer.empty();
}

QByteArray QtOpusInputProcessor::getNetworkPacket()
{
    return outputNetworkBuffer.takeFirst();
}

qint64 QtOpusInputProcessor::writeData(const char *data, qint64 maxSize)
{
    inputBuffer += QByteArray(data, maxSize);

    while((size_t)inputBuffer.size() > FRAME_SIZE * sizeof(opus_int16))
    {
        QByteArray source_frame = inputBuffer.left(FRAME_SIZE * sizeof(opus_int16));
        short* psMic = (short *)source_frame.data();

        EncodingOutputBuffer buffer;

        int nbBytes = opus_encode(encoder, psMic, FRAME_SIZE, &buffer[0], static_cast<opus_int32>(buffer.size()));

        QByteArray networkFrame(reinterpret_cast<char *>(&buffer[0]), nbBytes);
        outputNetworkBuffer.append(networkFrame);
        emit networkPacketReady();

        inputBuffer = inputBuffer.right(inputBuffer.size() - FRAME_SIZE * sizeof(opus_int16));
    }
    return maxSize;
}

/*
 *
 *
 *
 */

QtOpusOutputProcessor::QtOpusOutputProcessor(QObject *parent) : QIODevice(parent)
{
    int err;
    /* Create a new decoder state. */
    decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
    if (err<0)
       std::cerr << stderr << "failed to create decoder: %s\n" << opus_strerror(err) << std::endl;
}

QtOpusOutputProcessor::~QtOpusOutputProcessor()
{
    opus_decoder_destroy(decoder);
}

void QtOpusOutputProcessor::putNetworkPacket(QString name, QByteArray packet)
{
    inputNetworkBuffer.append(packet);
}

qint64 QtOpusOutputProcessor::readData(char *data, qint64 maxSize)
{
    while(outputBuffer.size() < maxSize)
    {
        if(!inputNetworkBuffer.isEmpty())
        {
            QByteArray buftmp = inputNetworkBuffer.takeFirst();
            QByteArray intermediate_frame;
            intermediate_frame.resize(FRAME_SIZE * sizeof(opus_int16));
            int frame_size = opus_decode(decoder, reinterpret_cast<unsigned char*>(buftmp.data()), buftmp.size(), (opus_int16*)intermediate_frame.data(), FRAME_SIZE, 0);
            outputBuffer += intermediate_frame;
        }
        else
            break;
    }

    QByteArray resultBuffer = outputBuffer.left(maxSize);

    memcpy(data, resultBuffer.data(), resultBuffer.size());

    outputBuffer = outputBuffer.right(outputBuffer.size() - resultBuffer.size());

    return resultBuffer.size();
}
