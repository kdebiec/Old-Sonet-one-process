/****************************************************************
 *  This file is part of Sonet.
 *  Sonet is distributed under the following license:
 *
 *  Copyright (C) 2006, crypton
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

//std
#include <unistd.h>
#include <iostream>

//Qt
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QObject>
#include <QMessageBox>

//libretroshare
#include <retroshare/rsinit.h>
#include <retroshare/rsiface.h>
#include <retroshare/rsfiles.h>
#include <rsserver/rsaccounts.h>
#include <util/argstream.h>

//Sonet-GUI
#include "notifytxt.h"
#include "sonetsettings.h"
#include "Bridge/VOIP/vnotify.h"
#include "Bridge/LoginWindow/loginwindow_main.h"
#include "Bridge/LoginWindow/loginwindow.h"
#include "Bridge/SoundManager.h"
#include "rsqml_main.h"

static void displayWarningAboutDSAKeys();

int main(int argc, char **argv)
{
    RsControl::earlyInitNotificationSystem() ;

    NotifyTxt *notify = NotifyTxt::Create() ;
    rsNotify->registerNotifyClient(notify);

    VNotify *vNotify = VNotify::Create();
    rsNotifyVOIP->registerNotifyClient(vNotify);



        /* RetroShare Core Objects */
    RsInit::InitRsConfig();

    int initResult = RsInit::InitRetroShare(argc, argv);

    if(initResult == RS_INIT_NO_KEYRING)	// happens when we already have accounts, but no pgp key. This is when switching to the openpgp-sdk version.
    {
        QMessageBox msgBox;
        msgBox.setText(QObject::tr("This version of RetroShare is using OpenPGP-SDK. As a side effect, it's not using the system shared PGP keyring, but has it's own keyring shared by all RetroShare instances. <br><br>You do not appear to have such a keyring, although PGP keys are mentioned by existing RetroShare accounts, probably because you just changed to this new version of the software."));
        msgBox.setInformativeText(QObject::tr("Choose between:<br><ul><li><b>Ok</b> to copy the existing keyring from gnupg (safest bet), or </li><li><b>Close without saving</b> to start fresh with an empty keyring (you will be asked to create a new PGP key to work with RetroShare, or import a previously saved pgp keypair). </li><li><b>Cancel</b> to quit and forge a keyring by yourself (needs some PGP skills)</li></ul>"));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ok);

        int ret = msgBox.exec();

        if(ret == QMessageBox::Cancel)
            return 0 ;

        if(ret == QMessageBox::Ok)
        {
            if(!RsAccounts::CopyGnuPGKeyrings())
                return 0 ;

            initResult = RsInit::InitRetroShare(argc, argv);

            displayWarningAboutDSAKeys() ;

        }
        else
            initResult = RS_INIT_OK ;
    }

    if (initResult < 0)
    {


        displayWarningAboutDSAKeys();

        QMessageBox mb(QMessageBox::Critical, QObject::tr("RetroShare"), "", QMessageBox::Ok);

        switch (initResult)
        {
            case RS_INIT_AUTH_FAILED:
                std::cerr << "RsInit::InitRetroShare AuthGPG::InitAuth failed" << std::endl;
                mb.setText(QObject::tr("Initialization failed. Wrong or missing installation of PGP."));
                break;
            default:
                /* Unexpected return code */
                std::cerr << "RsInit::InitRetroShare unexpected return code " << initResult << std::endl;
                mb.setText(QObject::tr("An unexpected error occurred. Please report 'RsInit::InitRetroShare unexpected return code %1'.").arg(initResult));
                break;
        }
        mb.exec();
        return 1;
    }

    switch (initResult)
    {
    case RS_INIT_OK:
        {
            /* Login Dialog */
            LoginWindow::Create();
            loginwindow_main(argc, argv);

        }
        break;
    case RS_INIT_HAVE_ACCOUNT:
        {

            RsPeerId preferredId;
            RsAccounts::GetPreferredAccountId(preferredId);
            LoginWindow::loadCertificate(preferredId, true);
        }
        break;
    default:
        /* Unexpected return code */
        std::cerr << "RsInit::InitRetroShare unexpected return code " << initResult << std::endl;
        QMessageBox::warning(0, QObject::tr("RetroShare"), QObject::tr("An unexpected error occured. Please report 'RsInit::InitRetroShare unexpected return code %1'.").arg(initResult));
        return 1;
    }

    SonetSettings::Create(true);

    SoundManager::create();

    /* stop Retroshare if startup fails */
    if (!RsControl::instance()->StartupRetroShare())
    {
        std::cerr << "libretroshare failed to startup!" << std::endl;
        return 1;
    }

    if (Settings->value(QString::fromUtf8("FirstRun"), true).toBool())
    {
        Settings->setValue(QString::fromUtf8("FirstRun"), false);
        SoundManager::initDefault();
    }

    rsFiles -> shareDownloadDirectory(false);

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    qRegisterMetaType<ChatMessage>("ChatMessage");
    qRegisterMetaType<MsgPreview>("MsgPreview");
    QObject::connect(notify,SIGNAL(deferredSignatureHandlingRequested()),notify,SLOT(handleSignatureEvent()),Qt::QueuedConnection);
    vNotify->enable();

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    rsqml_main(argc, argv);

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    RsControl::instance()->rsGlobalShutDown();

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    delete(soundManager);
    soundManager = NULL;

    Settings->sync();
    delete(Settings);
}


static void displayWarningAboutDSAKeys()
{
    std::map<std::string,std::vector<std::string> > unsupported_keys;
    RsAccounts::GetUnsupportedKeys(unsupported_keys);

    if(unsupported_keys.empty())
        return ;

    QMessageBox msgBox;

    QString txt = QObject::tr("You appear to have nodes associated to DSA keys:");
    txt += "<UL>" ;
    for(std::map<std::string,std::vector<std::string> >::const_iterator it(unsupported_keys.begin());it!=unsupported_keys.end();++it)
    {
        txt += "<LI>" + QString::fromStdString(it->first) ;
        txt += "<UL>" ;

        for(uint32_t i=0;i<it->second.size();++i)
            txt += "<li>" + QString::fromStdString(it->second[i]) + "</li>" ;

        txt += "</UL>" ;
        txt += "</li>" ;
    }
    txt += "</UL>" ;

    msgBox.setText(txt) ;
    msgBox.setInformativeText(QObject::tr("DSA keys are not yet supported by this version of RetroShare. All these nodes will be unusable. We're very sorry for that."));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);

    msgBox.exec();
}

