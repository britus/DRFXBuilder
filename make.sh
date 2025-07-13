#!/bin/bash
QT_PROJECT=DRFXBuilder.pro
XC_PROJECT=DRFXBuilder.xcodeproj
PROJECT_DIR=`pwd`
ARCH=`uname -m`

echo ":> Build project: ${ARCH} ..."

if [ ! -r ${PROJECT_DIR}/${QT_PROJECT} ] ; then
	echo "Oops! Wrong directory: ${PROJECT_DIR}"
	ls -la
	exit 1
fi

if [ -r ${PROJECT_DIR}/build/xcode/${XC_PROJECT} ] ; then
    cd build/xcode
    xcodebuild -arch `uname -m` -project ${XC_PROJECT} -target DRFXBuilder
else
    mkdir -p build/xcode
	echo "Generate Xcode project in `pwd`/build/xcode ..."
    cd build/xcode && \
        qmake -spec macx-xcode ../../${QT_PROJECT}
	cd build/xcode && \
		ln -s ../../DRFXBuilder_sanbox.entitlements DRFXBuilder.entitlements
    echo "!!! ---- [ NOTE ] ---- !!!"
    echo "Now you have to open Xcode IDE and setup the bundle identifier, signing and capabilities."
    echo "Also you have to add QT framework bundles to copy into app directory!."
    echo "Bootstrap done."
fi


