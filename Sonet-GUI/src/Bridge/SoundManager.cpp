/****************************************************************
 *  This file is part of Sonet.
 *  Sonet is distributed under the following license:
 *
 *  Copyright (C) 2012 RetroShare Team
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
#include "SoundManager.h"

//Qt
#include <QApplication>
#include <QFile>
#include <QProcess>
#include <QDir>

//libretroshare
#include <retroshare/rsinit.h>
#include <retroshare/rsplugin.h>

//Sonet-GUI
#include "sonetsettings.h"

#define GROUP_MAIN      "Sound"
#define GROUP_ENABLE    "Enable"
#define GROUP_SOUNDFILE "SoundFilePath"

SoundManager *soundManager = NULL;

void SoundEvents::addEvent(const QString &groupName, const QString &eventName, const QString &event, const QString &defaultFilename)
{
    if (event.isEmpty())
        return;

	SoundEventInfo info;
	info.mGroupName = groupName;
	info.mEventName = eventName;
	info.mDefaultFilename = defaultFilename;

	mEventInfos[event] = info;
}

void SoundManager::create()
{
    if (soundManager == NULL)
        soundManager = new SoundManager;
}

SoundManager::SoundManager() : QObject(), locked(false)
{
    initDefault();
}

void SoundManager::soundEvents(SoundEvents &events)
{
    QDir baseDir = QDir(QString::fromUtf8(RsAccounts::DataDirectory().c_str()) + "/sounds");

    events.mDefaultPath = baseDir.absolutePath();

    events.addEvent(tr("Message"), tr("Message sended"), SOUND_MESSAGE_SENDED, QFileInfo(baseDir, "msgsended.wav").absoluteFilePath());
    events.addEvent(tr("Message"), tr("Message received"), SOUND_MESSAGE_RECEIVED, QFileInfo(baseDir, "msgreceived.wav").absoluteFilePath());

    events.addEvent(tr("Call"), tr("Ringing from"), SOUND_RINGING, QFileInfo(baseDir, "ringing.wav").absoluteFilePath());
    events.addEvent(tr("Call"), tr("Calling to"), SOUND_CALLING, QFileInfo(baseDir, "dialing_tone_europe.wav").absoluteFilePath());
}

QString SoundManager::defaultFilename(const QString &event, bool check)
{
	SoundEvents events;
	soundEvents(events);

	QMap<QString, SoundEvents::SoundEventInfo>::iterator eventIt = events.mEventInfos.find(event);
    if (eventIt == events.mEventInfos.end())
        return "";

	QString filename = eventIt.value().mDefaultFilename;
    if (filename.isEmpty())
        return "";

    if (!check)
        return convertFilename(filename);

    if (QFileInfo(filename).exists())
        return convertFilename(filename);

	return "";
}

void SoundManager::initDefault()
{
	SoundEvents events;
	soundEvents(events);

	QString event;
    foreach (event, events.mEventInfos.keys())
    {
		SoundEvents::SoundEventInfo &eventInfo = events.mEventInfos[event];
        setEventFilename(event, convertFilename(eventInfo.mDefaultFilename));
	}
}

void SoundManager::setMute(bool m)
{
    Settings->beginGroup(GROUP_MAIN);
	Settings->setValue("mute", m);
	Settings->endGroup();

	emit mute(m);
}

bool SoundManager::isMute()
{
	Settings->beginGroup(GROUP_MAIN);
	bool mute = Settings->value("mute", false).toBool();
	Settings->endGroup();

	return mute;
}

QString SoundManager::eventFilename(const QString &event)
{
	Settings->beginGroup(GROUP_MAIN);
	Settings->beginGroup(GROUP_SOUNDFILE);
	QString filename = Settings->value(event).toString();
	Settings->endGroup();
	Settings->endGroup();

	return filename;
}

void SoundManager::setEventFilename(const QString &event, const QString &filename)
{
	Settings->beginGroup(GROUP_MAIN);
	Settings->beginGroup(GROUP_SOUNDFILE);
	Settings->setValue(event, filename);
	Settings->endGroup();
    Settings->endGroup();
}

QString SoundManager::convertFilename(const QString &filename)
{
    if (RsInit::isPortable ())
    {
		// Save path relative to application path
		QDir baseDir = QDir(qApp->applicationDirPath());
		QString relativeFilename = baseDir.relativeFilePath(filename);
        if (!relativeFilename.startsWith(".."))
            return relativeFilename;
	}

	return filename;
}

QString SoundManager::realFilename(const QString &filename)
{
    if (RsInit::isPortable ())
    {
		// Path relative to application path
		QDir baseDir = QDir(qApp->applicationDirPath());
		return baseDir.absoluteFilePath(filename);
	}

	return filename;
}

QSound* SoundManager::playEvent(const QString &event)
{
    QSound *qsound;
    QString filename = eventFilename(event);

    if (isMute() || filename.isEmpty())
        return qsound;

    QString playFilename = realFilename(filename);
    qsound = new QSound(playFilename);

    return qsound;
}

void SoundManager::play(const QString &event)
{
    if (isMute())
        return;

    QString filename = eventFilename(event);
    playFile(filename);
}

void SoundManager::playFile(const QString &filename)
{
    if (filename.isEmpty())
        return;

    QString playFilename = realFilename(filename);
    QSound::play(playFilename);
}
