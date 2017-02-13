#ifndef SONETSETTINGS_H
#define SONETSETTINGS_H
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

//Sonet-GUI
#include "rsettings.h"

class SonetSettings :  public RSettings
{
public:
    /* create settings object */
    static void Create(bool forceCreateNew = false);

public:
    bool getAdvancedMode();
    void setAdvancedMode(bool value);

    bool getFlickableGridMode();
    void setFlickableGridMode(bool value);

    bool getAutoLogin();
    void setAutoLogin(bool value);

protected:
    SonetSettings(){}
};

// the one and only global settings object
extern SonetSettings *Settings;

#endif // SONETSETTINGS_H
