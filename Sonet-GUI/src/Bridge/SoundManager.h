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
#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

//Qt
#include <QObject>
#include <QMap>
#include <QSound>

#define SOUND_MESSAGE_SENDED    "MessageSended"
#define SOUND_MESSAGE_RECEIVED  "MessageReceived"

#define SOUND_RINGING           "Ringing"
#define SOUND_CALLING           "Calling"

class SoundEvents
{
public:
	class SoundEventInfo
	{
	public:
		SoundEventInfo() {}

	public:
		QString mGroupName;
		QString mEventName;
		QString mDefaultFilename;
	};

public:
	void addEvent(const QString &groupName, const QString &eventName, const QString &event, const QString &defaultFilename);

	QString mDefaultPath;
	QMap<QString, SoundEventInfo> mEventInfos;
};

class SoundManager : public QObject
{
	Q_OBJECT

public slots:
	void setMute(bool m);

signals:
	void mute(bool isMute);

public:
	static void create();

#ifdef Q_OS_LINUX
    static QString soundDetectPlayer();
#endif

	static void initDefault();
	static QString defaultFilename(const QString &event, bool check);
	static QString convertFilename(const QString &filename);
	static QString realFilename(const QString &filename);

	static void soundEvents(SoundEvents &events);

	static bool isMute();

    void play(const QString &event);
    void playFile(const QString &filename);

    QSound* playEvent(const QString &event);

	static QString eventFilename(const QString &event);
	static void setEventFilename(const QString &event, const QString &filename);
    
private:
	SoundManager();

    bool locked;
};

extern SoundManager *soundManager;

#endif	//SOUNDMANAGER_H
