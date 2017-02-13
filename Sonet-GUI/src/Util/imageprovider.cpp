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
#include "imageprovider.h"

/*static*/ ImageProvider *ImageProvider::_instance = NULL;

/*static*/ ImageProvider *ImageProvider::Create ()
{
    if (_instance == NULL)
        _instance = new ImageProvider ();

    return _instance;
}
/*static*/ ImageProvider *ImageProvider::getInstance ()
{
    return _instance;
}

QPixmap ImageProvider::requestPixmap(const QString &path, QSize *size, const QSize& requestedSize)
{
    int slash = path.indexOf("/");
    QString prefix = path.left(slash);
    QString id = path;

    if(slash>0)
        id.remove(0, slash+1);

    int width = 512;
    int height = 512;

    if (size)
        *size = QSize(width, height);
    QPixmap pixmap(requestedSize.width() > 0 ? requestedSize.width() : width,
                   requestedSize.height() > 0 ? requestedSize.height() : height);

    if(prefix == "gxs")
    {
        /* get avatar */
        RsIdentityDetails details ;
        RsGxsId gxsId(id.toStdString());

        if(rsIdentity->getIdDetails(gxsId, details))
        {
            if(details.mAvatar.mSize == 0 || !pixmap.loadFromData(details.mAvatar.mData, details.mAvatar.mSize, "PNG"))
                pixmap = QPixmap(AVATAR_DEFAULT_IMAGE);
            return pixmap;
        }
    }
    else if(prefix == "wallavatar")
    {
        pixmap = QPixmap(WALLAVATAR_DEFAULT_IMAGE);
        return pixmap;
    }
    else if(prefix == "wallbg")
    {
        RsGxsId authorId(id.toStdString());

        uint32_t walltoken;
        RsTokReqOptions opts;
        opts.mReqType = GXS_REQUEST_TYPE_GROUP_DATA;
        mWallQueue->requestGroupInfo(walltoken, RS_TOKREQ_ANSTYPE_DATA, opts, 1);

        waitForTokenOrTimeout(walltoken, rsWall->getTokenService());

        std::vector<WallGroup> groups;
        if (!rsWall->getWallGroups(walltoken, groups))
            std::cerr << "ImageProvider::requestPixmap() Error getting WallGroups" << std::endl;

        for (std::vector<WallGroup>::iterator it = groups.begin() ; it != groups.end(); ++it)
        {
            if (it->mMeta.mAuthorId == authorId)
            {
                if(it->mWallImage.mData.size() == 0 || !pixmap.loadFromData(it->mWallImage.mData.data(), it->mWallImage.mData.size(), "PNG"))
                    pixmap = QPixmap(WALLBG_DEFAULT_IMAGE);
                return pixmap;
            }
        }

        pixmap = QPixmap(WALLBG_DEFAULT_IMAGE);
        return pixmap;
    }

    pixmap = QPixmap(AVATAR_DEFAULT_IMAGE);
    return pixmap;
}
