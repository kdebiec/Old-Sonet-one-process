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
#include "loginwindow.h"

//Qt
#include <QMessageBox>
#include <QApplication>

/*static*/ std::string LoginWindow::passwd;
/*static*/ LoginWindow *LoginWindow::_instance = NULL;

/*static*/ LoginWindow *LoginWindow::Create ()
{
    if (_instance == NULL)
        _instance = new LoginWindow ();

    return _instance;
}
/*static*/ LoginWindow *LoginWindow::getInstance ()
{
    return _instance;
}

LoginWindow::LoginWindow(QObject *parent) : QObject(parent)
{
    std::list<RsPeerId>::iterator it;

    if(RsAccounts::GetAccountIds(accountIds))
    {
        for (it = accountIds.begin(); it != accountIds.end(); ++it)
        {
            accountDetails details;
            RsAccounts::GetAccountDetails(*it, details.gpgid, details.name,  details.email, details.node);
            accountDetailsList.push_back(details);
            QString accountName = QString::fromUtf8(details.name.c_str());
            accname << accountName;
        }
    }
}

const QStringList LoginWindow::loadAccounts()
{
    return accname;
}

bool LoginWindow::loadPerson(QString password, int index, bool autoLogin)
{
    savePass(password.toStdString());
    std::list<RsPeerId>::iterator it = accountIds.begin();
    int i=0;

    while (i != index)
    {
        i++;
        it++;
    }

    if (!RsAccounts::SelectAccount(*it))
    {
        std::cerr << "Can't select account" << std::endl;
        return false;
    }

    std::string lockFile;
    int retVal = RsInit::LockAndLoadCertificates(autoLogin, lockFile);
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
//		case 3: QMessageBox::critical(	0,
//										QObject::tr("Login Failure"),
//										QObject::tr("Maybe password is wrong") );
                return false;
        default: std::cerr << "LoginWindow::loadPerson() unexpected switch value " << retVal << std::endl;
                return false;
    }

    emit closeWindow();

    return true;
}

bool LoginWindow::loadCertificate(const RsPeerId &accountId, bool autoLogin)
{
    if (!RsAccounts::SelectAccount(accountId))
        return false;

    std::string lockFile;
    int retVal = RsInit::LockAndLoadCertificates(autoLogin, lockFile);
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
//		case 3: QMessageBox::critical(	0,
//										QObject::tr("Login Failure"),
//										QObject::tr("Maybe password is wrong") );
                return false;
        default: std::cerr << "Rshare::loadCertificate() unexpected switch value " << retVal << std::endl;
                return false;
    }

    return true;
}

int LoginWindow::preferedAccount()
{
    return pidx;
}

int LoginWindow::numberOfAccounts()
{
    return accountIds.size();
}
/*static*/ std::string LoginWindow::getPass()
{
    return passwd;
}

/*static*/ void LoginWindow::savePass(QString pass)
{
    passwd = pass.toUtf8().constData();
}

/*static*/ void LoginWindow::savePass(std::string pass)
{
    passwd = pass;
}
