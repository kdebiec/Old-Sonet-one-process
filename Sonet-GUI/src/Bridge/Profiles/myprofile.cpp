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
#include "myprofile.h"

//Qt
#include <QMessageBox>
#include <QTimer>
#include <QBuffer>
#include <QFileDialog>
#include <QDir>

//libretroshare
#include <retroshare/rsidentity.h>
#include <services/wall/rswall.h>
#include <retroshare/rsstatus.h>
#include <gxs/rsgixs.h>

#define MYPROFILE_LOADID   1
#define MYPROFILE_CREATEID 2
#define MYPROFILE_GRP      3
// ///////////////////////////
#define GXSGROUP_NEWGROUPID         4
// ///////////////////////////
#define MYPROFILE_GETGRP     5

/*** Group flags affect what is visually enabled that gets input into the grpMeta ***/

#define GXS_GROUP_FLAGS_NAME			0x00000001
#define GXS_GROUP_FLAGS_ICON			0x00000002
#define GXS_GROUP_FLAGS_DESCRIPTION		0x00000004
#define GXS_GROUP_FLAGS_DISTRIBUTION		0x00000008
#define GXS_GROUP_FLAGS_PUBLISHSIGN		0x00000010
#define GXS_GROUP_FLAGS_SHAREKEYS		0x00000020
#define GXS_GROUP_FLAGS_PERSONALSIGN		0x00000040
#define GXS_GROUP_FLAGS_COMMENTS		0x00000080
#define GXS_GROUP_FLAGS_EXTRA			0x00000100
#define GXS_GROUP_FLAGS_ANTI_SPAM   		0x00000200

/*** Default flags are used to determine privacy of group, signatures required ***
 *** whether publish or id and whether comments are allowed or not             ***/

#define GXS_GROUP_DEFAULTS_DISTRIB_MASK	0x0000000f
#define GXS_GROUP_DEFAULTS_PUBLISH_MASK	0x000000f0
#define GXS_GROUP_DEFAULTS_PERSONAL_MASK	0x00000f00
#define GXS_GROUP_DEFAULTS_COMMENTS_MASK	0x0000f000

#define GXS_GROUP_DEFAULTS_DISTRIB_PUBLIC	0x00000001
#define GXS_GROUP_DEFAULTS_DISTRIB_GROUP	0x00000002
#define GXS_GROUP_DEFAULTS_DISTRIB_LOCAL	0x00000004

#define GXS_GROUP_DEFAULTS_PUBLISH_OPEN	0x00000010
#define GXS_GROUP_DEFAULTS_PUBLISH_THREADS	0x00000020
#define GXS_GROUP_DEFAULTS_PUBLISH_REQUIRED	0x00000040
#define GXS_GROUP_DEFAULTS_PUBLISH_ENCRYPTED	0x00000080

#define GXS_GROUP_DEFAULTS_PERSONAL_PGP	0x00000100
#define GXS_GROUP_DEFAULTS_PERSONAL_REQUIRED	0x00000200
#define GXS_GROUP_DEFAULTS_PERSONAL_IFNOPUB	0x00000400

#define GXS_GROUP_DEFAULTS_COMMENTS_YES	0x00001000
#define GXS_GROUP_DEFAULTS_COMMENTS_NO		0x00002000

#define GXS_GROUP_DEFAULTS_ANTISPAM_FAVOR_PGP		0x00100000
#define GXS_GROUP_DEFAULTS_ANTISPAM_TRACK		0x00200000
#define GXS_GROUP_DEFAULTS_ANTISPAM_FAVOR_PGP_KNOWN	0x00400000

using namespace RsWall;

bool waitForTokenOrTimeout(uint32_t token, RsTokenService* tokenService)
{
    time_t start = time(NULL);
    while(   (tokenService->requestStatus(token) != RsTokenService::GXS_REQUEST_V2_STATUS_COMPLETE)
          && (tokenService->requestStatus(token) != RsTokenService::GXS_REQUEST_V2_STATUS_FAILED)
          && (time(NULL) < (start+10))
          )
    {
#ifdef WINDOWS_SYS
        Sleep(500);
#else
        usleep(500*1000) ;
#endif
    }
    if(tokenService->requestStatus(token) == RsTokenService::GXS_REQUEST_V2_STATUS_COMPLETE)
        return true;
    else
        return false;
}

/*static*/ MyProfile *MyProfile::_instance = NULL;

/*static*/ MyProfile *MyProfile::Create()
{
    if (_instance == NULL)
        _instance = new MyProfile();

    return _instance;
}

/*static*/ void MyProfile::Destroy()
{
    if(_instance != NULL)
        delete _instance ;

    _instance = NULL ;
}

/*static*/ MyProfile *MyProfile::getInstance()
{
    return _instance;
}

MyProfile::MyProfile(QObject *parent) : QObject(parent)
{
    QObject::connect(NotifyTxt::getInstance(), SIGNAL(passwordForIdentityAccepted()), this, SLOT(createWall()));
    QObject::connect(NotifyTxt::getInstance(), SIGNAL(passwordForIdentityRejected()), this, SLOT(firstIdentityRejected()));

    mIdQueue = new TokenQueue(rsIdentity->getTokenService(), this);
    mWallQueue = new TokenQueue(rsWall->getTokenService(), this);

    rsIdentity->getOwnIds(own_identities);
    qTimer = new QTimer();
    qTimer->setInterval(1000);
    qTimer->setSingleShot(true);
    QObject::connect(qTimer, SIGNAL(timeout()), this, SLOT(createWall()));
}

MyProfile::~MyProfile()
{
    delete mIdQueue;
    delete mWallQueue;

    if(qTimer != NULL)
        delete qTimer;

    qTimer = NULL;
}

void MyProfile::firstIdentityRejected()
{
    emit firstIdentityCancelled();
}

QString MyProfile::showMyCert()
{
    std::string cert = rsPeers->GetRetroshareInvite(false);
    QString mycert = QString::fromStdString(cert.c_str());
    return mycert;
}

bool MyProfile::haveIdentity()
{
    return !own_identities.empty();
}

QString MyProfile::getNickBaseIdentity()
{
    rsIdentity->getOwnIds(own_identities);
    if(!own_identities.empty())
    {
        time_t start = time(NULL);
        // have to try to get the identity details multiple times, until they are cached
        bool ok  = false;
        do
        {
            std::list<RsGxsId>::const_iterator gxsIt = own_identities.begin();
            for (gxsIt; gxsIt != own_identities.end(); ++gxsIt)
            {
                RsIdentityDetails details;
                ok = rsIdentity->getIdDetails(*gxsIt, details);
                if(ok && !details.mNickname.empty())
                    return QString::fromUtf8(details.mNickname.c_str());
            }
#ifdef WINDOWS_SYS
            Sleep(500);
#else
            usleep(500*1000);
#endif
        } while(!ok && (time(NULL)< (start+10)));
    }
    return QString("");
}

QString MyProfile::getPreferredGxs()
{
    rsIdentity->getOwnIds(own_identities);
    if(!own_identities.empty())
    {
        std::list<RsGxsId>::const_iterator gxsIt = own_identities.begin();

        time_t start = time(NULL);
        bool ok = false;
        do
        {
            gxsIt = own_identities.begin();
            for (gxsIt; gxsIt != own_identities.end(); ++gxsIt)
            {
                RsIdentityDetails details;
                ok = rsIdentity->getIdDetails(*gxsIt, details);
                if(ok && !details.mNickname.empty())
                    break;
            }
#ifdef WINDOWS_SYS
            Sleep(500);
#else
            usleep(500*1000);
#endif
        } while(!ok && (time(NULL)< (start+10)));

        RsGxsId id = *gxsIt;

        if(id.isNull())
            return QString();

        return QString::fromStdString(id.toStdString());
    }
    return QString();
}

bool MyProfile::addIdentity(QString nickname, QString src)
{
    if (nickname.size() < 2)
    {
        std::cerr << "MyProfile::addIdentity() Nickname too short";
        std::cerr << std::endl;
        QMessageBox msgBox;
        msgBox.setText("The nickname is too short.");
        msgBox.setInformativeText("Please input at least 2 characters.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();

        emit firstIdentityCancelled();
        return false;
    }
    else if (nickname.size() > RSID_MAXIMUM_NICKNAME_SIZE)
    {
        std::cerr << "MyProfile::addIdentity() Nickname too long (max " << RSID_MAXIMUM_NICKNAME_SIZE<< " chars)";
        std::cerr << std::endl;

        QMessageBox msgBox;
        msgBox.setText("The nickname is too long.");
        msgBox.setInformativeText("Please reduce the length to 30 characters.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();

        emit firstIdentityCancelled();
        return false;
    }

    RsIdentityParameters params;
    params.nickname = nickname.toUtf8().constData();
    params.isPgpLinked = true;

    if(src == "")
    {
        QString file = src.remove(0, 8);
        int width = 512;
        int height = 512;

#ifdef WINDOWS_SYS
        // fix bug in Qt for Windows Vista and higher, convert path from native separators
        if (QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA && (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based))
            file = QDir::fromNativeSeparators(file);
#endif

        QPixmap mAvatar = QPixmap(file).scaledToHeight(height, Qt::SmoothTransformation).copy( 0, 0, width, height);

        if (!mAvatar.isNull())
        {
            QByteArray ba;
            QBuffer buffer(&ba);

            buffer.open(QIODevice::WriteOnly);
            mAvatar.save(&buffer, "PNG"); // writes image into ba in PNG format

            params.mImage.copy((uint8_t *) ba.data(), ba.size());
        }
        else
            params.mImage.clear();

    }
    uint32_t token = 0;

    rsIdentity->createIdentity(token, params);
    mIdQueue->queueRequest(token, 0, 0, MYPROFILE_CREATEID);

    return true;
}

void MyProfile::createWall()
{
    rsIdentity->getOwnIds(own_identities);

    while(own_identities.empty())
    {
        rsIdentity->getOwnIds(own_identities);
        Sleep(500);
    }

    std::list<RsGxsId>::iterator gxsIt = own_identities.begin();

    bool nextTry = true;

    for (gxsIt; gxsIt != own_identities.end(); ++gxsIt)
    {
        RsIdentityDetails details;
        int ok;
        ok = rsIdentity->getIdDetails(*gxsIt, details);

        if(ok)
        {
            if(!details.mNickname.empty())
            {
                nextTry = false;
                break;
            }
        }
    }

    if(nextTry)
    {
        qTimer->start();
        return;
    }

    WallGroup wg;
    wg.mMeta.mAuthorId = *gxsIt;
    wg.mMeta.mGroupName = "WallService";
    uint32_t CreateDefaultsFlags = ( GXS_GROUP_DEFAULTS_DISTRIB_PUBLIC    |
                                        //GXS_GROUP_DEFAULTS_DISTRIB_GROUP        |
                                        //GXS_GROUP_DEFAULTS_DISTRIB_LOCAL        |

                                        GXS_GROUP_DEFAULTS_PUBLISH_OPEN         |
                                        //GXS_GROUP_DEFAULTS_PUBLISH_THREADS      |
                                        //GXS_GROUP_DEFAULTS_PUBLISH_REQUIRED     |
                                        //GXS_GROUP_DEFAULTS_PUBLISH_ENCRYPTED    |

                                        //GXS_GROUP_DEFAULTS_PERSONAL_GPG         |
                                        GXS_GROUP_DEFAULTS_PERSONAL_REQUIRED    |
                                        //GXS_GROUP_DEFAULTS_PERSONAL_IFNOPUB     |

                                        GXS_GROUP_DEFAULTS_COMMENTS_YES         |
                                        //GXS_GROUP_DEFAULTS_COMMENTS_NO          |
                                        0);

    wg.mMeta.mGroupFlags = GXS_SERV::FLAG_PRIVACY_PUBLIC;
    wg.mMeta.mSignFlags = CreateDefaultsFlags;

    wg.mMeta.mCircleType = GXS_CIRCLE_TYPE_PUBLIC;//GXS_CIRCLE_TYPE_YOUR_FRIENDS_ONLY;
    wg.mMeta.mCircleId.clear();
    wg.mMeta.mOriginator.clear();
    wg.mMeta.mInternalCircle.clear();

    uint32_t token2;
    rsWall->createWallGroup(token2, wg);
    mWallQueue->queueRequest(token2, TOKENREQ_GROUPINFO, RS_TOKREQ_ANSTYPE_ACK, GXSGROUP_NEWGROUPID);

    emit firstIdentityCreated();
}

void MyProfile::setAvatar(/*QString gxs*/)
{
    int width = 512;
    int height = 512;

    QString file;
    file = QFileDialog::getOpenFileName();

    if (file.isEmpty())
        return;

#ifdef WINDOWS_SYS
    // fix bug in Qt for Windows Vista and higher, convert path from native separators
    if (QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA && (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based))
        file = QDir::fromNativeSeparators(file);
#endif

    std::cerr << file.toStdString() << std::endl;

    RsGxsIdGroup grp;

    RsTokReqOptions opts;
    opts.mReqType = GXS_REQUEST_TYPE_GROUP_DATA;

    uint32_t token;

    mIdQueue->requestGroupInfo(token, RS_TOKREQ_ANSTYPE_DATA, opts, MYPROFILE_GETGRP);

    waitForTokenOrTimeout(token, rsIdentity->getTokenService());

    std::vector<RsGxsIdGroup> datavector;
    if (!rsIdentity->getGroupData(token, datavector))
    {
        std::cerr << "Error getting key!" << std::endl;
        return;
    }

    std::vector<RsGxsIdGroup>::size_type sz = datavector.size();

    grp = datavector[(sz-1)];
    QPixmap avatar = QPixmap(file).scaledToHeight(height, Qt::SmoothTransformation).copy( 0, 0, width, height);

    if (avatar.isNull())
    {
        std::cerr << "Image is empty" << std::endl;
        return;
    }

    if (!avatar.isNull())
    {
        QByteArray ba;
        QBuffer buffer(&ba);

        buffer.open(QIODevice::WriteOnly);
        avatar.save(&buffer, "PNG"); // writes image into ba in PNG format

        grp.mImage.copy((uint8_t *) ba.data(), ba.size());
    }
    else
        grp.mImage.clear();

    uint32_t dummyToken = 0;
    rsIdentity->updateIdentity(dummyToken, grp);
}

void MyProfile::setWallBg()
{
    int width = 1024;
    int height = 1024;

    QString file;
    file = QFileDialog::getOpenFileName();

    if (file.isEmpty())
        return;

#ifdef WINDOWS_SYS
    // fix bug in Qt for Windows Vista and higher, convert path from native separators
    if (QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA && (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based))
        file = QDir::fromNativeSeparators(file);
#endif

    uint32_t walltoken;
    rsIdentity->getOwnIds(own_identities);
    std::list<RsGxsId>::const_iterator gxsIt = own_identities.begin();
    RsGxsId author = *gxsIt;
    if(author.isNull())
        return;

    RsTokReqOptions opts;
    opts.mReqType = GXS_REQUEST_TYPE_GROUP_DATA;
    mWallQueue->requestGroupInfo(walltoken, RS_TOKREQ_ANSTYPE_DATA, opts, MYPROFILE_GETGRP);

    waitForTokenOrTimeout(walltoken, rsWall->getTokenService());

    std::vector<WallGroup> groups;
    if (!rsWall->getWallGroups(walltoken, groups))
    {
        std::cerr << "MyProfile::getCountGxs() Error getting WallGroups" << std::endl;
        return;
    }

    std::vector<WallGroup>::iterator wg = groups.begin();

    for (std::vector<WallGroup>::iterator it = groups.begin() ; it != groups.end(); ++it)
    {
        if (it->mMeta.mAuthorId == author)
            *wg = *it;
    }

    QPixmap avatar = QPixmap(file).scaledToHeight(height, Qt::SmoothTransformation).copy( 0, 0, width, height);

    if (avatar.isNull())
    {
        std::cerr << "Image is empty" << std::endl;
        return;
    }

    if (!avatar.isNull())
    {
        QByteArray ba;
        QBuffer buffer(&ba);

        buffer.open(QIODevice::WriteOnly);
        avatar.save(&buffer, "PNG"); // writes image into ba in PNG format

        uint8_t * src = (uint8_t *)ba.data();
        wg->mWallImage.mData.clear();
        wg->mWallImage.mData.insert(wg->mWallImage.mData.begin(), src, src + ba.size());

        uint32_t token = 0;
        rsWall->updateWallGroup(token, *wg);
    }
}

void MyProfile::changeStatus()
{
    StatusInfo statusInfo;
    if (rsStatus->getOwnStatus(statusInfo))
    {
        uint32_t status = statusInfo.status;
        if (status == RS_STATUS_BUSY)
            status = RS_STATUS_ONLINE;
        else if (status == RS_STATUS_AWAY)
            status = RS_STATUS_BUSY;
        else if (status == RS_STATUS_ONLINE)
            status = RS_STATUS_AWAY;
        rsStatus->sendStatus(RsPeerId(), status);
    }
}

QColor MyProfile::getStatus()
{
    StatusInfo statusInfo;
    if (rsStatus->getOwnStatus(statusInfo))
    {
        if(statusInfo.status == RS_STATUS_ONLINE)
            return QColor("#4caf50"); //green
        else if(statusInfo.status == RS_STATUS_BUSY)
            return QColor("#FF5722"); //red
        else if(statusInfo.status == RS_STATUS_AWAY)
            return QColor("#FFEB3B"); //yellow
        else
            return QColor(0, 0, 0, 0.38); //grey
    }
    else
        return QColor(0, 0, 0, 0.38); //grey
}

void MyProfile::setStatusMsg(QString msg)
{
    rsMsgs->setCustomStateString(msg.toStdString());
}

QString MyProfile::getStatusMsg()
{
    QString msg = QString::fromStdString(rsMsgs->getCustomStateString());
    if (!msg.isEmpty())
        return msg;

    return QString("");
}
