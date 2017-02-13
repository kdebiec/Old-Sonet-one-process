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

#include "sonetsettings.h"

//libretroshare
#include <retroshare/rsinit.h>

// the one and only global settings object
SonetSettings *Settings = NULL;

/*static*/ void SonetSettings::Create(bool forceCreateNew)
{
    if (Settings && (forceCreateNew || Settings->m_bValid == false))
    {
        // recreate with correct path
        delete (Settings);
        Settings = NULL;
    }
    if (Settings == NULL)
        Settings = new SonetSettings ();
}

bool SonetSettings::getAdvancedMode()
{
    return valueFromGroup("General", "Advanced", false).toBool();
}

void SonetSettings::setAdvancedMode(bool value)
{
    setValueToGroup("General", "Advanced", value);
}

bool SonetSettings::getFlickableGridMode()
{
    return valueFromGroup("General", "FlickableGrid", false).toBool();
}

void SonetSettings::setFlickableGridMode(bool value)
{
    setValueToGroup("General", "FlickableGrid", value);
}

bool SonetSettings::getAutoLogin()
{
    return RsInit::getAutoLogin();
}

void SonetSettings::setAutoLogin(bool value)
{
    RsInit::setAutoLogin(value);
}
