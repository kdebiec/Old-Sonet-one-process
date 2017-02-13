!include("sonet.pri"): error("Could not include file sonet.pri")

INCLUDEPATH += $$PWD/Build/libs/include
DEPENDPATH += $$PWD/Build/libs/include

TEMPLATE = subdirs

CONFIG+= static

SUBDIRS += \
        openpgpsdk \
        libbitdht \
        libretroshare \
        Sonet-GUI/src/Sonet-GUI.pro

openpgpsdk.file = openpgpsdk/src/openpgpsdk.pro
libbitdht.file = libbitdht/src/libbitdht.pro
libretroshare.file = libretroshare/src/libretroshare.pro

libretroshare.depends = openpgpsdk libbitdht
