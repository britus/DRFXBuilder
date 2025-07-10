QT += core gui widgets

win32:TEMPLATE = vcapp
else:TEMPLATE = app

TARGET = drfxbuilder

win32:VERSION = 0.1.3.2 # major.minor.patch.build
else:VERSION = 0.1.3    # major.minor.patch

# Defaults
CONFIG += c++17
CONFIG += sdk_no_version_check
CONFIG += global_init_link_order
CONFIG += incremental
CONFIG += debug_and_release
CONFIG += force_debug_info

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


#otool -L
LIBS += -dead_strip
LIBS += -liconv

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
	QMAKE_RPATHDIR += @executable_path/../Plugins
	QMAKE_RPATHDIR += @executable_path/../lib

	LICENSE.files = $$PWD/LICENSE
	LICENSE.path = Contents/Resources
	QMAKE_BUNDLE_DATA += LICENSE

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
		$$PWD/assets/icns/drfxbuilder.icns \
		$$PWD/assets/png/drfxbuilder.png
	icons.path = Contents/Resources/
	QMAKE_BUNDLE_DATA += icons

	QMAKE_INFO_PLIST += \
		$$PWD/Info.plist

	info_plist.files = \
		$$PWD/Info.plist
	info_plist.path = Contents/
	QMAKE_BUNDLE_DATA += info_plist

	QMAKE_CODE_SIGN_ENTITLEMENTS=$$PWD/app.entitlements
	QMAKE_CODE_SIGN_IDENTITY='Mac Developer'
}

# Default rules for deployment.
mac {
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

SOURCES += \
	main.cpp \
	mainwindow.cpp

HEADERS += \
	mainwindow.h

FORMS += \
	mainwindow.ui

TRANSLATIONS += \
	DRFXBuilder_en_US.ts

RESOURCES += \
	DRFXBuilder.qrc

DISTFILES += \
	LICENSE \
	README.md \
	deploy.sh \
	qmake-options.txt
