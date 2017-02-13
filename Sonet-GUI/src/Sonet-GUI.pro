!include("../../sonet.pri"): error("Could not include file ../../sonet.pri")

DEFINES += QPM_INIT\\(E\\)=\"E.addImportPath(QStringLiteral(\\\"qrc:/\\\"));\"
include(../../qml-material/qml-material-develop/material.pri)


QT += qml quick
QT += widgets quickwidgets
QT += multimedia

CONFIG += c++11 qrc bitdht
CONFIG += link_prl

TEMPLATE = app
TARGET = Sonet

DEPENDPATH *= Sonet-GUI
INCLUDEPATH *= Sonet-GUI


#DEFINES += PGPLISTMODEL_DEBUG
#DEFINES += NOTIFY_DEBUG
#DEFINES += VNOTIFY_DEBUG




#################################### Windows #####################################

win32 {
        #if you want to compile borderless login window
        #just define this variables
        {
            DEFINES += BORDERLESS_LOGIN
            DEFINES += BORDERLESS_MAINWINDOW

            QT += gui-private
            LIBS += -ldwmapi

            HEADERS += \
                Bridge/Windows/qwinview.h \
                Bridge/MainWindow/mainwindow.h \
                Bridge/MainWindow/mainwindowpanel.h \
                Bridge/LoginWindow/QMainPanel.h \
                Bridge/LoginWindow/borderlesswindow.h

            SOURCES += \
                Bridge/Windows/qwinview.cpp \
                Bridge/MainWindow/mainwindow.cpp \
                Bridge/MainWindow/mainwindowpanel.cpp \
                Bridge/LoginWindow/QMainPanel.cpp \
                Bridge/LoginWindow/borderlesswindow.cpp
        }

        OBJECTS_DIR = temp/obj

        LIBS_DIR = $$PWD/../../libs/lib
        LIBS += $$OUT_PWD/../../libretroshare/src/lib/libretroshare.a
        LIBS += $$OUT_PWD/../../openpgpsdk/src/lib/libops.a

        for(lib, LIB_DIR):LIBS += -L"$$lib"
        for(bin, BIN_DIR):LIBS += -L"$$bin"


        LIBS += -lssl -lcrypto -lpthread -lminiupnpc -lz -lws2_32
        LIBS += -luuid -lole32 -liphlpapi -lcrypt32 -lgdi32
        LIBS += -lwinmm
        LIBS += -lopus
        #LIBS += -lspeex -lspeexdsp

        DEFINES *= WINDOWS_SYS WIN32_LEAN_AND_MEAN _USE_32BIT_TIME_T

        DEPENDPATH += . $$INC_DIR
        INCLUDEPATH += . $$INC_DIR

        greaterThan(QT_MAJOR_VERSION, 4) {
                # Qt 5
                RC_INCLUDEPATH += $$_PRO_FILE_PWD_/../../libretroshare/src
        } else {
                # Qt 4
                QMAKE_RC += --include-dir=$$_PRO_FILE_PWD_/../../libretroshare/src
        }
}

################################### COMMON stuff ##################################

DEPENDPATH += . ../../libretroshare/src/
INCLUDEPATH += ../../libretroshare/src/

####################--BASE--####################

HEADERS += \
    rsqml_main.h \
    notifytxt.h \
    Util/TokenQueue.h \
    Bridge/VOIP/audiodevicehelper.h \
    Bridge/VOIP/opusprocessor.h \
    Bridge/VOIP/vnotify.h \
    Util/imageprovider.h \
    sonetsettings.h \
    rsettings.h \
    Bridge/ssettings.h \
    Bridge/pgplistmodel.h \
    Bridge/wallcommentmodel.h \
    Util/screensize.h \
    Bridge/PopUp/popup.h \
    Util/cursorshape.h \
    Util/qquickviewhelper.h \
    Bridge/msgstore.h \
    Bridge/statuscolor.h

SOURCES += main.cpp \
    rsqml_main.cc \
    notifytxt.cc \
    Util/TokenQueue.cpp \
    Bridge/VOIP/audiodevicehelper.cpp \
    Bridge/VOIP/opusprocessor.cpp \
    Bridge/VOIP/vnotify.cpp \
    sonetsettings.cpp \
    rsettings.cpp \
    Bridge/ssettings.cpp \
    Bridge/pgplistmodel.cpp \
    Bridge/wallcommentmodel.cpp \
    Bridge/PopUp/popup.cpp \
    Util/imageprovider.cpp \
    Bridge/msgstore.cpp \
    Bridge/statuscolor.cpp

################################################
###################--BRIDGE--###################

HEADERS += \
    Bridge/Chat/chat.h \
    Bridge/Chat/chathandler.h \
    Bridge/Chat/chatmsgmodel.h \
    Bridge/LoginWindow/loginwindow.h \
    Bridge/LoginWindow/loginwindow_main.h \
    Bridge/Profiles/myprofile.h \
    Bridge/friendlist.h \
    Bridge/gencert.h \
    Bridge/wallpostmodel.h \
    Bridge/gxsidmodel.h \
    Bridge/VOIP/voip.h \
    Bridge/VOIP/voiphandler.h \
    Bridge/SoundManager.h

SOURCES += \
    Bridge/Chat/chat.cpp \
    Bridge/Chat/chathandler.cpp \
    Bridge/Chat/chatmsgmodel.cpp \
    Bridge/LoginWindow/loginwindow.cpp \
    Bridge/LoginWindow/loginwindow_main.cpp \
    Bridge/Profiles/myprofile.cpp \
    Bridge/friendlist.cpp \
    Bridge/gencert.cpp \
    Bridge/wallpostmodel.cpp \
    Bridge/gxsidmodel.cpp \
    Bridge/VOIP/voip.cpp \
    Bridge/VOIP/voiphandler.cpp \
    Bridge/SoundManager.cpp

################################################

# Default rules for deployment.
include(deployment.pri)

RESOURCES += \
    GUI/LoginWindow/login.qrc \
    GUI/MainWindow/mainwindow.qrc \
    GUI/PopUp/popup.qrc \
    Images/images.qrc
