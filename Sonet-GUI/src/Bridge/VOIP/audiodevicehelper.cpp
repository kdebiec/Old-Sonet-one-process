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
#include "audiodevicehelper.h"

QAudioInput* AudioDeviceHelper::getDefaultInputDevice() 
{
    QAudioFormat fmt;
#if QT_VERSION >= QT_VERSION_CHECK (5, 0, 0)
    fmt.setSampleRate(16000);
    fmt.setChannelCount(1);
#else
    fmt.setFrequency(16000);
    fmt.setChannels(1);
#endif
    fmt.setSampleSize(16);
    fmt.setSampleType(QAudioFormat::SignedInt);
    fmt.setByteOrder(QAudioFormat::LittleEndian);
    fmt.setCodec("audio/pcm");

    QAudioDeviceInfo it, dev;
	 QList<QAudioDeviceInfo> input_list = QAudioDeviceInfo::availableDevices(QAudio::AudioInput) ;

    dev = QAudioDeviceInfo::defaultInputDevice();
    if (dev.deviceName() != "pulse")
    {
        foreach(it, input_list)
        {
            if(it.deviceName() == "pulse")
            {
                dev = it;
                qDebug("Ok.");
                break;
            }
        }
    }
    if (dev.deviceName() == "null")
    {
        foreach(it, input_list)
        {
            if(it.deviceName() != "null")
            {
                dev = it;
                break;
            }
        }
    }
    return new QAudioInput(dev, fmt);
}

QAudioInput* AudioDeviceHelper::getPreferedInputDevice()
{
    return AudioDeviceHelper::getDefaultInputDevice();
}

QAudioOutput* AudioDeviceHelper::getDefaultOutputDevice()
{
    QAudioFormat fmt;
#if QT_VERSION >= QT_VERSION_CHECK (5, 0, 0)
    fmt.setSampleRate(16000);
    fmt.setChannelCount(1);
#else
    fmt.setFrequency(16000);
    fmt.setChannels(1);
#endif
    fmt.setSampleSize(16);
    fmt.setSampleType(QAudioFormat::SignedInt);
    fmt.setByteOrder(QAudioFormat::LittleEndian);
    fmt.setCodec("audio/pcm");

	 QList<QAudioDeviceInfo> list_output = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput) ;

    QAudioDeviceInfo it, dev;
    dev = QAudioDeviceInfo::defaultOutputDevice();

    if (dev.deviceName() != "pulse")
    {
        foreach(it, list_output)
        {
            if(it.deviceName() == "pulse")
            {
                dev = it;
                break;
            }
        }
    }
    if (dev.deviceName() == "null")
    {
        foreach(it, list_output)
        {
            if(it.deviceName() != "null")
            {
                dev = it;
                break;
            }
        }
    }
    return new QAudioOutput(dev, fmt);
}

QAudioOutput* AudioDeviceHelper::getPreferedOutputDevice()
{
    return AudioDeviceHelper::getDefaultOutputDevice();
}
