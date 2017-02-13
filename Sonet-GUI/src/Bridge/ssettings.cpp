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
#include "ssettings.h"

//Sonet-GUI
#include "sonetsettings.h"

/*static*/ SSettings *SSettings::_instance = NULL;

/*static*/ SSettings *SSettings::Create ()
{
    if (_instance == NULL)
        _instance = new SSettings ();

    return _instance;
}

/*static*/ void SSettings::Destroy()
{
    if(_instance != NULL)
        delete _instance ;

    _instance = NULL ;
}

/*static*/ SSettings *SSettings::getInstance ()
{
    return _instance;
}

bool SSettings::getAdvancedMode()
{
    return Settings->getAdvancedMode();
}

void SSettings::setAdvancedMode(bool value)
{
    Settings->setAdvancedMode(value);
}

bool SSettings::getFlickableGridMode()
{
    return Settings->getFlickableGridMode();
}

void SSettings::setFlickableGridMode(bool value)
{
    Settings->setFlickableGridMode(value);
}

bool SSettings::getAutoLogin()
{
    return Settings->getAutoLogin();
}

void SSettings::setAutoLogin(bool value)
{
    Settings->setAutoLogin(value);
}
