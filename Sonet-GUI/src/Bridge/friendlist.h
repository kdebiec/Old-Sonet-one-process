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
#ifndef FRIENDLIST_H
#define FRIENDLIST_H

//Qt
#include <QObject>

//libretroshare
#include <retroshare/rsidentity.h>
#include <retroshare/rspeers.h>

class FriendList :  public QObject
{
    Q_OBJECT
public:
    static FriendList *Create ();
    static void Destroy();
    static FriendList *getInstance ();

public slots:
    void addFriend(QString friendcert);     //Add rspeers to contacts
    void addContact(RsGxsId &id);           //Add gxsid to contacts
    void removeContact(RsGxsId &id);        //Remove gxsid from contacts
    void removeGxsContact(QString gxsid);     //Remove Gxs friend from contacts
    void removePgpContact(QString pgpid);     //Remove Pgp friend from contacts

private:
    FriendList(QObject *parent = 0) : QObject(parent){}
    static FriendList *_instance;

    RsPeerDetails peerDetails;
    std::string mCertificate;
};

#endif // FRIENDLIST_H
