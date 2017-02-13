# To disable SQLCipher support append the following assignation to qmake
# command line "CONFIG+=no_sqlcipher"
CONFIG *= sqlcipher
no_sqlcipher:CONFIG -= sqlcipher

# To enable autologin (this is higly discouraged as it may compromise your node
# security in multiple ways) append the following assignation to qmake command
# line "CONFIG+=rs_autologin"
CONFIG *= no_rs_autologin
rs_autologin:CONFIG -= no_rs_autologin
CONFIG+=rs_autologin

# To disable GXS (General eXchange System) append the following
# assignation to qmake command line "CONFIG+=no_rs_gxs"
CONFIG *= rs_gxs
no_rs_gxs:CONFIG -= rs_gxs

# To disable Deprecated Warning append the following
# assignation to qmake command line "CONFIG+=rs_nodeprecatedwarning"
CONFIG *= no_rs_nodeprecatedwarning
rs_nodeprecatedwarning:CONFIG -= no_rs_nodeprecatedwarning
CONFIG+=rs_nodeprecatedwarning

# To disable Cpp #Warning append the following
# assignation to qmake command line "CONFIG+=rs_nocppwarning"
CONFIG *= no_rs_nocppwarning
rs_nocppwarning:CONFIG -= no_rs_nocppwarning
CONFIG+=rs_nocppwarning

# Gxs is always enabled now.
DEFINES *= RS_ENABLE_GXS

# Wall service is always enabled now.
DEFINES *= RS_ENABLE_WALL

# VOIP service is always enabled now.
DEFINES *= RS_ENABLE_VOIP
#DEFINES *= DEBUG_VOIP

unix {
	isEmpty(PREFIX)   { PREFIX   = "/usr" }
	isEmpty(BIN_DIR)  { BIN_DIR  = "$${PREFIX}/bin" }
	isEmpty(INC_DIR)  { INC_DIR  = "$${PREFIX}/include/retroshare06" }
	isEmpty(LIB_DIR)  { LIB_DIR  = "$${PREFIX}/lib" }
	isEmpty(DATA_DIR) { DATA_DIR = "$${PREFIX}/share/RetroShare06" }
	isEmpty(PLUGIN_DIR) { PLUGIN_DIR  = "$${LIB_DIR}/retroshare/extensions6" }

    rs_autologin {
        !macx {
            DEFINES *= HAS_GNOME_KEYRING
            PKGCONFIG *= gnome-keyring-1
        }
    }
}

win32 {
        message(***sonet.pri:Win32)
        exists($$PWD/Build/libs) {
		message(Get pre-compiled libraries.)
                isEmpty(PREFIX)   { PREFIX   = "$$PWD/Build/libs" }
		isEmpty(BIN_DIR)  { BIN_DIR  = "$${PREFIX}/bin" }
		isEmpty(INC_DIR)  { INC_DIR  = "$${PREFIX}/include" }
		isEmpty(LIB_DIR)  { LIB_DIR  = "$${PREFIX}/lib" }
	}
        # Check for msys2
        PREFIX_MSYS2 = $$(MINGW_PREFIX)
        isEmpty(PREFIX_MSYS2) {
                exists(C:/msys32/mingw32/include) {
                        message(MINGW_PREFIX is empty. Set it in your environment variables.)
                        message(Found it here:C:\msys32\mingw32)
                        PREFIX_MSYS2 = "C:\msys32\mingw32"
                }
                exists(C:/msys64/mingw32/include) {
                        message(MINGW_PREFIX is empty. Set it in your environment variables.)
                        message(Found it here:C:\msys64\mingw32)
                        PREFIX_MSYS2 = "C:\msys64\mingw32"
                }
        }
        !isEmpty(PREFIX_MSYS2) {
                message(msys2 is installed.)
                BIN_DIR  += "$${PREFIX_MSYS2}/bin"
                INC_DIR  += "$${PREFIX_MSYS2}/include"
                LIB_DIR  += "$${PREFIX_MSYS2}/lib"
        }
        CONFIG += c+11
}

macx {
        message(***sonet.pri:MacOSX)
	BIN_DIR += "/usr/bin"
	INC_DIR += "/usr/include"
	INC_DIR += "/usr/local/include"
	INC_DIR += "/opt/local/include"
	LIB_DIR += "/usr/local/lib"
	LIB_DIR += "/opt/local/lib"
        !QMAKE_MACOSX_DEPLOYMENT_TARGET {
                message(***sonet.pri: No Target. Set it to MacOS 10.11 )
                QMAKE_MACOSX_DEPLOYMENT_TARGET=10.11
        }
        !QMAKE_MAC_SDK {
                message(***sonet.pri: No SDK. Set it to MacOS 10.11 )
                QMAKE_MAC_SDK = macosx10.11
        }
	CONFIG += c+11
}

unfinished {
	CONFIG += gxscircles
	CONFIG += gxsthewire
	CONFIG += gxsphotoshare
	CONFIG += wikipoos
}
CONFIG += c+11

rs_gxs:DEFINES *= RS_ENABLE_GXS
sqlcipher:DEFINES -= NO_SQLCIPHER
no_sqlcipher:DEFINES *= NO_SQLCIPHER

rs_autologin {
    DEFINES *= RS_AUTOLOGIN
    warning("You have enabled RetroShare auto-login, this is discouraged. The usage of auto-login on some linux distributions may allow someone having access to your session to steal the SSL keys of your node location and therefore compromise your security")
}

rs_nodeprecatedwarning {
    QMAKE_CXXFLAGS += -Wno-deprecated
    QMAKE_CXXFLAGS += -Wno-deprecated-declarations
    DEFINES *= RS_NO_WARN_DEPRECATED
    warning("QMAKE: You have disable deprecated warnings.")
}

rs_nocppwarning {
    QMAKE_CXXFLAGS += -Wno-cpp
    DEFINES *= RS_NO_WARN_CPP
    warning("QMAKE: You have disable cpp warnings.")
}
