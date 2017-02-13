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
#include "friendlist.h"

//std
#include <iostream>

//Sonet-GUI
#include "notifytxt.h"

/*static*/ FriendList *FriendList::_instance = NULL;

/*static*/ FriendList *FriendList::Create()
{
    if (_instance == NULL)
        _instance = new FriendList();

    return _instance;
}

/*static*/ void FriendList::Destroy()
{
    if(_instance != NULL)
        delete _instance ;

    _instance = NULL ;
}

/*static*/ FriendList *FriendList::getInstance()
{
    return _instance;
}

void FriendList::addContact(RsGxsId &id)
{
    rsIdentity->setAsRegularContact(id,true);
}

void FriendList::removeContact(RsGxsId &id)
{
    rsIdentity->setAsRegularContact(id,false);
}

void FriendList::removeGxsContact(QString gxsid)
{
    RsGxsId rsgxs(gxsid.toStdString());
    RsIdentityDetails details;
    rsIdentity->getIdDetails(rsgxs, details);
    if(details.mPgpId.toStdString() != "")
        rsPeers->removeFriend(details.mPgpId);
    rsIdentity->setAsRegularContact(rsgxs,false);
}

void FriendList::removePgpContact(QString pgpid)
{
    RsPgpId rspgp(pgpid.toStdString());
    rsPeers->removeFriend(rspgp);
}

void FriendList::addFriend(QString friendcert)
{
    std::string certstr = friendcert.toStdString();
    uint32_t cert_load_error_code;

    if (rsPeers->loadDetailsFromStringCert(certstr, peerDetails, cert_load_error_code))
        mCertificate = certstr;
    else
        std::cerr << "friend Certificate Load Failed - something goes wrong" << std::endl;

    bool sign = true;
    bool accept_connection = true;
    bool add_key_to_keyring = true;

    if (!mCertificate.empty() && add_key_to_keyring)
    {
        RsPgpId pgp_id ;
        RsPeerId ssl_id ;
        std::string error_string ;

        if(!rsPeers->loadCertificateFromString(mCertificate,ssl_id,pgp_id,error_string))
        {
            std::cerr << "FriendList::addFriend(QString friendcert): cannot load that certificate." << std::endl;
            return;
        }
    }

    ServicePermissionFlags flags(0) ;

    if(accept_connection && !peerDetails.gpg_id.isNull())
    {
        std::cerr << "FriendList::addFriend(QString friendcert) accepting GPG key for connection." << std::endl;
        rsPeers->addFriend(peerDetails.id, peerDetails.gpg_id, flags) ;
        rsPeers->setServicePermissionFlags(peerDetails.gpg_id, flags) ;
        if(sign)
        {
            std::cerr << "FriendList::addFriend(QString friendcert) signing GPG key." << std::endl;
            rsPeers->signGPGCertificate(peerDetails.gpg_id); //bye default sign set accept_connection to true;
            rsPeers->setServicePermissionFlags(peerDetails.gpg_id,flags) ;
        }
    }
    if ((accept_connection) && (!peerDetails.id.isNull()))
    {
        if (!peerDetails.location.empty()) {
            std::cerr << "FriendList::addFriend(QString friendcert) : setting peer node." << std::endl;
            rsPeers->setLocation(peerDetails.id, peerDetails.location);
        }

        if (peerDetails.isHiddenNode)
        {
            std::cerr << "FriendList::addFriend(QString friendcert) : setting hidden node." << std::endl;
            rsPeers->setHiddenNode(peerDetails.id, peerDetails.hiddenNodeAddress, peerDetails.hiddenNodePort);
        }
        else
        {
            //let's check if there is ip adresses in the wizard.
            if (!peerDetails.extAddr.empty() && peerDetails.extPort) {
                std::cerr << "FriendList::addFriend(QString friendcert) : setting ip ext address." << std::endl;
                rsPeers->setExtAddress(peerDetails.id, peerDetails.extAddr, peerDetails.extPort);
            }
            if (!peerDetails.localAddr.empty() && peerDetails.localPort) {
                std::cerr << "FriendList::addFriend(QString friendcert) : setting ip local address." << std::endl;
                rsPeers->setLocalAddress(peerDetails.id, peerDetails.localAddr, peerDetails.localPort);
            }
            if (!peerDetails.dyndns.empty()) {
                std::cerr << "FriendList::addFriend(QString friendcert) : setting DynDNS." << std::endl;
                rsPeers->setDynDNS(peerDetails.id, peerDetails.dyndns);
            }
        }
    }
    NotifyTxt::getInstance()->notifyListChange(NOTIFY_LIST_NEIGHBOURS,1);
    NotifyTxt::getInstance()->notifyListChange(NOTIFY_LIST_FRIENDS,1);
}
