#!/bin/bash
MYPWD=`basename`
DEVID=`cat $0/.devid`
APP="build/DRFXBuilder.app"

find . -name ".DS*" -exec rm {} ";"
xattr -cr .

if [ "x${1}" == "x" ] ; then
	echo "Using QT build: ${APP}"
else
	APP="${1}"
	echo "Using build: ${APP}"
fi

${QTDIR}/bin/macdeployqt ${APP} -always-overwrite -timestamp -appstore-compliant -codesign=${DEVID}
