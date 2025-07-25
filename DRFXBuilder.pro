QT += core gui widgets

windows:TEMPLATE = vcapp
else:TEMPLATE = app

TARGET = DRFXBuilder

windows:VERSION = 1.0.3.20 # major.minor.patch.build
else:VERSION = 1.0.3    # major.minor.patch

# Defaults
CONFIG += c++17
CONFIG += sdk_no_version_check
CONFIG += global_init_link_order
CONFIG += incremental
CONFIG += debug_and_release
#CONFIG += force_debug_info

# debug info in executable
CONFIG -= separate_debug_info

# strip all symbols
#CONFIG += nostrip

# no library info
CONFIG -= create_prl

# for translations
#CONFIG += lrelease

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

# for QT Creator and Visual Studio project.
QMAKE_PROJECT_NAME = $${TARGET}

QMAKE_CXXFLAGS += -fno-omit-frame-pointer
QMAKE_CXXFLAGS += -funwind-tables

release {
	#QMAKE_LFLAGS += -s
}

unix:debug {
	QMAKE_CXXFLAGS += -ggdb3
}

# QZip support
windows {
	QMAKE_INCDIR += $$PWD/qzip/src/3rdparty/zlib
	INCLUDEPATH += $$PWD/qzip/src/3rdparty/zlib
	include($$PWD/qzip/src/3rdparty/zlib.pri)

	#SOURCES += \
	#	$$PWD/qzip/src/compress/qzip.cpp

	#HEADERS += \
	#	$$PWD/qzip/src/compress/qzipreader.h \
	#	$$PWD/qzip/src/compress/qzipwriter.h
}

unix{
	INCLUDEPATH += $$PWD/qzip/src/compress
	QMAKE_INCDIR += /usr/local/include
	QMAKE_LIBDIR += /usr/local/lib
	QMAKE_LIBS += -lz
}

mac {
	lessThan(QT_MAJOR_VERSION, 6) {
		QT += macextras
	}

	CONFIG += app_bundle
	CONFIG += embed_libraries
	CONFIG += embed_translations

	QMAKE_CFLAGS += -mmacosx-version-min=12.2
	QMAKE_CXXFLAGS += -mmacosx-version-min=12.2

	QMAKE_MACOSX_DEPLOYMENT_TARGET = 12.2

	# Important for the App with embedded frameworks and libs
	QMAKE_RPATHDIR += @executable_path/../Frameworks
	QMAKE_RPATHDIR += @executable_path/../PlugIns
	QMAKE_RPATHDIR += @executable_path/../lib

	LICENSE.files = $$PWD/LICENSE
	LICENSE.path = Contents/Resources
	QMAKE_BUNDLE_DATA += LICENSE

	#otool -L
	LIBS += -dead_strip
	LIBS += -liconv

	#translations_en.files = \
	#	$$PWD/assets/en.lproj/InfoPlist.strings \
	#	$$PWD/vspui_en_US.qm
	#translations_en.path = Contents/Resources/en.lproj
	#QMAKE_BUNDLE_DATA += translations_en

	#translations_de.files = \
	#	$$PWD/assets/de.lproj/InfoPlist.strings \
	#	$$PWD/vspui_de_DE.qm
	#translations_de.path = Contents/Resources/de.lproj
	#QMAKE_BUNDLE_DATA += translations_de

	icons.files = \
		$$PWD/assets/icns/drfxbuilder_1024x1024.icns \
		$$PWD/assets/icns/drfxbuilder_128x128.icns \
		$$PWD/assets/icns/drfxbuilder_16x16.icns \
		$$PWD/assets/icns/drfxbuilder_256x256.icns \
		$$PWD/assets/icns/drfxbuilder_32x32.icns \
		$$PWD/assets/icns/drfxbuilder_512x512.icns \
		$$PWD/assets/icns/drfxbuilder_64x64.icns \
		$$PWD/assets/icns/drfxbuilder.icns \
		$$PWD/assets/drfxbuilder.iconset/icon_128x128.png \
		$$PWD/assets/drfxbuilder.iconset/icon_128x128@2x.png \
		$$PWD/assets/drfxbuilder.iconset/icon_16x16.png \
		$$PWD/assets/drfxbuilder.iconset/icon_16x16@2x.png \
		$$PWD/assets/drfxbuilder.iconset/icon_256x256.png \
		$$PWD/assets/drfxbuilder.iconset/icon_256x256@2x.png \
		$$PWD/assets/drfxbuilder.iconset/icon_32x32.png \
		$$PWD/assets/drfxbuilder.iconset/icon_32x32@2x.png \
		$$PWD/assets/drfxbuilder.iconset/icon_512x512.png \
		$$PWD/assets/drfxbuilder.iconset/icon_512x512@2x.png \
		$$PWD/assets/png/drfxbuilder.png
	icons.path = Contents/Resources/
	QMAKE_BUNDLE_DATA += icons

	QMAKE_INFO_PLIST += \
		$$PWD/Info.plist

	info_plist.files = \
		$$PWD/Info.plist
	info_plist.path = Contents/
	QMAKE_BUNDLE_DATA += info_plist

	QMAKE_CODE_SIGN_ENTITLEMENTS=$$PWD/DRFXBuilder_no_sandbox.entitlements
	QMAKE_CODE_SIGN_IDENTITY='Mac Developer'

	target.path = /Applications
	INSTALLS += target
}

linux {
	target.path = /usr/local/bin
	INSTALLS += target
}

windows {
	target.path = $${HOME}
	INSTALLS += target
}

mac {
	OBJECTIVE_SOURCES += $$PWD/drfxsandbox.mm
	OBJECTIVE_HEADERS += $$PWD/drfxsandbox.h
}

SOURCES += \
	$$PWD/drfxbuilder.cpp \
	$$PWD/main.cpp \
	$$PWD/mainwindow.cpp \
	$$PWD/qzip/src/compress/qzip.cpp \
	$$PWD/drfxprogressdialog.cpp

HEADERS += \
	$$PWD/drfxbuilder.h \
	$$PWD/drfxtypes.h \
	$$PWD/mainwindow.h \
	$$PWD/qzip/src/compress/qtcompressglobal.h \
	$$PWD/qzip/src/compress/qzipreader.h \
	$$PWD/qzip/src/compress/qzipwriter.h \
	$$PWD/drfxprogressdialog.h

FORMS += \
	$$PWD/mainwindow.ui \
	$$PWD/drfxprogressdialog.ui

TRANSLATIONS += \
	$$PWD/DRFXBuilder_en_US.ts

RESOURCES += \
	$$PWD/DRFXBuilder.qrc

DISTFILES += \
	$$PWD/DRFXBuilder_no_sandbox.entitlements \
	$$PWD/DRFXBuilder_sandbox.entitlements \
	$$PWD/LICENSE \
	$$PWD/README.md \
	$$PWD/appstyle.css \
	$$PWD/appstyleqt5.css \
	$$PWD/deploy.sh \
	$$PWD/DRFXBuilder.lua \
	$$PWD/make.sh \
	$$PWD/privacy.txt \
	$$PWD/qmake-options.txt \
	$$PWD/xcode-macdeployqt.txt \
	$$PWD/xcode_script_env.txt \
	$$PWD/xcode_script_qt5.sh \
	$$PWD/xcode_script_qt6.sh
