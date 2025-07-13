#!/bin/bash
DEVID=`cat .devid`
APP="build/DRFXBuilder.app"

if [ "x${1}" == "x" ] ; then
	echo "Using QT build: ${APP}"
else
	APP="${1}"
	echo "Using build: ${APP}"
fi

macdeployqt ${APP} -always-overwrite -timestamp -appstore-compliant -codesign=${DEVID}
