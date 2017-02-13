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
#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

//Qt
#include <QQuickImageProvider>

//libretroshare
#include <retroshare/rsidentity.h>
#include <services/wall/p3wallservice.h>

//Sonet-GUI
#include "util/TokenQueue.h"

#define AVATAR_DEFAULT_IMAGE ":/avatar.jpg"
#define WALLAVATAR_DEFAULT_IMAGE ":/avatar.jpg"
#define WALLBG_DEFAULT_IMAGE ":/colorful.jpg"

using namespace RsWall;

class ImageProvider : public QQuickImageProvider, public TokenResponse
{
public:
    static ImageProvider *Create ();
    static ImageProvider *getInstance ();
    virtual ~ImageProvider(){}

    virtual QPixmap requestPixmap(const QString &path, QSize *size, const QSize& requestedSize);
    void loadRequest(const TokenQueue*, const TokenRequest&){}

private:
    ImageProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap)
    {
        mIdQueue = new TokenQueue(rsIdentity->getTokenService(), this);
        mWallQueue = new TokenQueue(rsWall->getTokenService(), this);
    }

    TokenQueue *mIdQueue;
    TokenQueue *mWallQueue;

    static ImageProvider *_instance;
};

bool waitForTokenOrTimeout(uint32_t token, RsTokenService* tokenService);

#endif // IMAGEPROVIDER_H
