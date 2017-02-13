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
#include "gencert.h"

//Qt
#include <QMessageBox>

//libretroshare
#include <retroshare/rsinit.h>
#include <rsserver/rsaccounts.h>

//Sonet-GUI
#include "LoginWindow/loginwindow.h"

bool GenCert::genPerson(QString name, QString passwd, QString passwd2, QString node)
{
    std::string genLoc  = node.toStdString();
    RsPgpId PGPId;
    bool isHiddenLoc = false;
    if (passwd.length() < 3 || name.length() < 3 || genLoc.length() < 3)
    {
        std::cerr << "All fields are required with a minimum of 3 characters" << std::endl;
        return false;
    }
    if(passwd != passwd2)
    {
        std::cerr << "Password do not match" << std::endl;
        return false;
    }

    LoginWindow::getInstance()->savePass(passwd);

    std::string err_string;
    std::string email_str = "" ;
    RsAccounts::GeneratePGPCertificate(
                name.toStdString(),
                email_str.c_str(),
                passwd.toStdString(),
                PGPId,
                2048,//4096,
                err_string);
    std::string sslPasswd = RSRandom::random_alphaNumericString(RsInit::getSslPwdLen()) ;
    RsPeerId sslId;
    std::cerr << "GenCert::genPerson() Generating SSL cert with gpg id : " << PGPId << std::endl;
    std::string err;
    bool okGen = RsAccounts::GenerateSSLCertificate(PGPId, "", genLoc, "", isHiddenLoc, sslPasswd, sslId, err);

    if (okGen)
    {
        /* complete the process */
        RsInit::LoadPassword(sslPasswd);
        if (!RsAccounts::SelectAccount(sslId))
            return false;

        std::string lockFile;
        int retVal = RsInit::LockAndLoadCertificates(false, lockFile);
        switch (retVal)
        {
            case 0:	break;
            case 1:	QMessageBox::warning(	0,
                                            QObject::tr("Multiple instances"),
                                            QObject::tr("Another RetroShare using the same profile is "
                                            "already running on your system. Please close "
                                            "that instance first\n Lock file:\n") +
                                            QString::fromUtf8(lockFile.c_str()));
                    return false;
            case 2:	QMessageBox::critical(	0,
                                            QObject::tr("Multiple instances"),
                                            QObject::tr("An unexpected error occurred when Retroshare "
                                            "tried to acquire the single instance lock\n Lock file:\n") +
                                            QString::fromUtf8(lockFile.c_str()));
                    return false;
            case 3:
                    return false;
            default: std::cerr << "Rshare::loadCertificate() unexpected switch value " << retVal << std::endl;
                    return false;
        }
        LoginWindow::getInstance()->closeWindow();
        return true;

    }
    return false;
}
