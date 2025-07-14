#!/bin/bash
DEVID=`cat .devid`
APP="build/DRFXBuilder.app"

if [ "x${QTDIR}" == "x" ] ; then
	echo "Set QTDIR=<where your QT platform is installed>"
	exit 1
fi

find . -name ".DS*" -exec rm {} ";"
xattr -cr .

if [ "x${1}" == "x" ] ; then
	echo "Using QT build: ${APP}"
else
	APP="${1}"
	echo "Using build: ${APP}"
fi

${QTDIR}/bin/macdeployqt ${APP} -always-overwrite -timestamp -appstore-compliant -codesign=${DEVID}
