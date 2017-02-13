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
#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

//Qt
#include <QObject>
#include <QStringList>
#include <QApplication>

//libretroshare
#include "retroshare/rsinit.h"

class LoginWindow : public QObject
{
    Q_OBJECT
public:

    static LoginWindow *Create ();
    static LoginWindow *getInstance ();

    static void savePass(QString pass);
    static bool loadCertificate(const RsPeerId &accountId, bool autoLogin);

signals:
    void closeWindow(); // zamykanie okna

public slots:
    const QStringList loadAccounts();
    bool loadPerson(QString password, int index, bool autoLogin = false);
    int preferedAccount();
    int numberOfAccounts();

    static std::string getPass();

private:
    explicit LoginWindow(QObject *parent = 0);
    static void savePass(std::string pass);
    static LoginWindow *_instance;
    static std::string passwd;

    struct accountDetails{
        RsPgpId gpgid;
        std::string name, email, node;
    };

    int pidx;
    std::list<RsPeerId> accountIds;
    std::list<accountDetails> accountDetailsList;
    QStringList accname;
    RsPeerId preferedId;
};

#endif // LOGINWINDOW_H
