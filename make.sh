#!/bin/bash
QT_PROJECT=DRFXBuilder.pro
XC_PROJECT=DRFXBuilder.xcodeproj

PROJECT_DIR=`pwd`
XC_PRJ_DIR=${PROJECT_DIR}/xcode
ARCH=`uname -m`

echo ":> Build project: ${ARCH} ..."

if [ ! -r ${PROJECT_DIR}/${QT_PROJECT} ] ; then
    echo "Oops! Wrong directory: ${PROJECT_DIR}"
    ls -la
    exit 1
fi

if [ -r ${XC_PRJ_DIR}/${XC_PROJECT} ] ; then
    find . -name ".DS*" -exec rm {} ";"
    cd ${XC_PRJ_DIR}
    xattr -cr .
    xcodebuild -arch `uname -m` -project ${XC_PROJECT} -target DRFXBuilder
else
    echo "Generate Xcode project in ${PROJECT_DIR} ..."
    mkdir -p ${XC_PRJ_DIR}
    cd ${XC_PRJ_DIR}
    qmake -spec macx-xcode ${PROJECT_DIR}/${QT_PROJECT}
    rm -f ${XC_PRJ_DIR}/DRFXBuilder.entitlements ${XC_PRJ_DIR}/Info.plist
    ln -fs ${PROJECT_DIR}/DRFXBuilder_sandbox.entitlements DRFXBuilder.entitlements
    ln -fs ${PROJECT_DIR}/Info.plist Info.plist
    echo "!!! ---- [ NOTE ] ---- !!!"
    echo "- Now you have to open Xcode IDE and setup the bundle identifier,"
    echo "  framework/plugin dependencies, signing and capabilities."
    echo "- Set custom build variable QTDIR in Xcode to your QT installation."
    echo "- Add the custom build script xcode_script_copy to build phases."
    echo "- Embedd required QT framework bundles."
    echo "Bootstrap done."
fi


